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

	FourierTransform(int historySize = 1, float cutOffLow = 0.0f, float cutOffHigh = 22050.0f, float cutoffSmoothFrac = 0.0f);
	~FourierTransform();
	
	void init(Master* master);
	void reInit(Master* master);

	void beginCalculation(); //applying filters must be between begin and end

	void endCalculation();

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
	FourierTransformHistory* getHistory() { return _current; }
	float* getOutput() const { return _current->newest(); }
	int getNumHarmonics() const { return _current->numHarmonics(); }

	float* getLowResOutput() const { return _lowResOutputLogScale; }
	int getLowResOutputSize() const { return _lowResOutputSize; }

	float getCutoffLow() const { return _cutOffLow; }
	float getCutoffHigh() const { return _cutOffHigh; }
	float getCutoffSmoothFraction() const { return _cutoffSmoothFrac; }

	Master* getMasterPtr() const { return _m; }
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

	void commonInit();

	//called by init
	void initDefaultVars();

	//used when getting the cut fourier transform
	float smoothCutoff(int i);
};

