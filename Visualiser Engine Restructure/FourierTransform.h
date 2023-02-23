#pragma once
#include "Master.h"
#include "FourierTransformHistory.h"

class FourierTransform {
public:
	static enum FunctionType {
		TIME_CONVOLVE,
		SMOOTH,
		FREQUENCY_CONVOLVE
	};

	FourierTransform(int historySize = 1, float cutOffLow = 0.0f, float cutOffHigh = 22050.0f, float cutoffSmoothFrac = 0.0f) :

		_cutOffLow(cutOffLow),
		_cutOffHigh(cutOffHigh),
		_cutoffSmoothFrac(cutoffSmoothFrac),

		_working1(historySize),
		_working2(historySize),

		_historySize(historySize),
		_initialised(false),

		_numHarmonics(-1)
	{
		_maxLowResOutputSize = 50;

	}

	~FourierTransform()
	{
		if (_initialised) {
			delete[] _smoothedFt;
			delete[] _smoothedFtDot;
		}
 	}
	
	void init(Master* master){
		_initialised = true;
		_m = master;

		initDefaultVars();

		//calculate cutoff points
		float fourierLowFrac = _cutOffLow * 2.0f / float(_m->_sampleRate); //as nyquist max freq
		float fourierHighFrac = _cutOffHigh * 2.0f / float(_m->_sampleRate);
		if (fourierHighFrac > 1) {
			fourierHighFrac = 1;
		}
		_harmonicLow = floorf(fourierLowFrac * float(_m->_fftHistory.numHarmonics()));
		_harmonicHigh = ceilf(fourierHighFrac * float(_m->_fftHistory.numHarmonics()));

		//check values work
		if (_harmonicHigh >= _m->_fftHistory.numHarmonics()) {
			_harmonicHigh = _m->_fftHistory.numHarmonics() - 1;
		}
		if (_harmonicLow < 0) {
			_harmonicLow = 0;
		}

		//calculate num harmonics
		_numHarmonics = (_harmonicHigh - _harmonicLow) + 1;

		_working1.init(_numHarmonics);
		_working2.init(_numHarmonics);

		_current = &_working1;
		_next = &_working2;

		//needed for smoothing
		_smoothedFt = new float[_numHarmonics];
		memset(_smoothedFt, 0.0f, sizeof(float) * _numHarmonics);

		_smoothedFtDot = new float[_numHarmonics];
		memset(_smoothedFtDot, 0.0f, sizeof(float) * _numHarmonics);

		//low res spectrum vars
		_lowResOutputSize = std::min(_maxLowResOutputSize, _numHarmonics);
		_lowResOutputLogScale = new float[_lowResOutputSize];
		memset(_lowResOutputLogScale, 0.0f, _lowResOutputSize * sizeof(float));
	}

	void beginCalculation() //applying filters must be between begin and end
	{
		//reset which is current and next at start of calculations
		_current = &(_working1);
		_next = &(_working2);

		if (!_initialised) {
			Vengine::fatalError("calculate called before initialise, banded fourier transform");
		}

		//get the fourier transform cutoff at selected frequency
		int index = 0;
		for (int i = _harmonicLow; i <= _harmonicHigh; i++) {
			_current->workingArray()[index] = _m->_fftHistory.newest()[i] * smoothCutoff(i);
			index++;
		}
	}

	void endCalculation() {
		_current->addWorkingArrayToHistory(_m->_currentSample);

		//do not worry about lots of variables, c++ compiler optimises away
		//calculate low res output
		memset(_lowResOutputLogScale, 0.0f, _lowResOutputSize * sizeof(float));

		for (int i = 0; i < _numHarmonics; i++) {

			int index = int(powf(float(i) / float(_numHarmonics), 2) * float(_numHarmonics));
			int outIndex = int((float(i) / float(_numHarmonics)) * float(_lowResOutputSize));

			_lowResOutputLogScale[outIndex] += _current->newest()[index];
		}
	}

	void applyFunction(FunctionType type); //must be called in every frame to work
	void applyFunctions(FunctionType* args, int numArgs);

	//for setting functions for ft vars--
	void setFrequencyConvolvingVars(int windowSize, Kernel kernel = LINEAR_PYRAMID) {
		_windowSize = windowSize; _freqKernel = kernel;
	}

	void setSmoothVars(float attack, float release, float maxAccelerationPerSecond) {
		_attack = attack; _release = release; _maxAccelerationPerSecond = maxAccelerationPerSecond;
	}

	void setTimeConvolvingVars(int previousXtransform, Kernel kernel) {
		_previousXtransforms = previousXtransform; _timeKernel = kernel;
	}
	//--

	//getters
	FourierTransformHistory* getHistory() {
		return _current;
	}

	float* getOutput() {
		return _current->newest();
	}

	float* getLowResOutput() {
		return _lowResOutputLogScale;
	}

	int getLowResOutputSize() {
		return _lowResOutputSize;
	}

	float getCutoffLow() {
		return _cutOffLow;
	}

	float getCutoffHigh() {
		return _cutOffHigh;
	}

	int getNumHarmonics() {
		return _current->numHarmonics();
	}

private:
	FourierTransformHistory* _current;
	FourierTransformHistory* _next;

	FourierTransformHistory _working1;
	FourierTransformHistory _working2;

	int _maxLowResOutputSize;
	int _lowResOutputSize;
	float* _lowResOutputLogScale;

	bool _initialised;
	Master* _m;
	int _numHarmonics;
	int _historySize;

	//cutoff vars
	float _cutOffLow, _cutOffHigh;
	int _harmonicLow, _harmonicHigh;
	float _cutoffSmoothFrac;


	//transform vars--
	//frequency convolving vars
	int _windowSize;
	Kernel _freqKernel;

	//smooth vars
	float _attack, _release, _maxAccelerationPerSecond;
	float* _smoothedFt; //store seperately as need to remember smoothed array how is was before another filter applied maybe
	float* _smoothedFtDot;

	//time convolve vars
	int _previousXtransforms;
	Kernel _timeKernel;
	//--

	//apply functions to ft--
	//defined in .cpp
	void applySmoothing(float* in, float* current);
	void applyTimeConvolving(FourierTransformHistory* in, float* out);
	void applyFreqConvolving(float* in, float* out);
	//--

	//called by init
	void initDefaultVars() {
		//default freq convolve vars
		_windowSize = 5;
		_freqKernel = LINEAR_PYRAMID;

		//default smooth vars
		_attack = 0.15;
		_release = 0.5;
		_maxAccelerationPerSecond = 0.5;

		//default time convolve vars
		_previousXtransforms = std::min(_historySize, 7);
		_timeKernel = LINEAR_PYRAMID;
	}

	//used when getting the cut fourier transform
	float smoothCutoff(int i) {
		if (_cutoffSmoothFrac == 0.0f) {
			return 1.0f; //no smoothing
		}

		float distanceFromCutoffFrac = float(std::min(i - _harmonicLow, _harmonicHigh - i)) / float(_numHarmonics);

		return std::min((2.0f * distanceFromCutoffFrac) / _cutoffSmoothFrac, 1.0f); //1.0f => pyramid band, 0.5f => trapezium with top side half of bottom, 0.1f => trapezium top side 9/10 of bottom
	}

};

