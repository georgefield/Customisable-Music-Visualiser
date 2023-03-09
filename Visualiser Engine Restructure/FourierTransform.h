#pragma once
#include "Master.h"
#include "FourierTransformHistory.h"
#include "Energy.hpp"

class FourierTransform {
public:
	static enum FunctionType {
		TIME_CONVOLVE,
		SMOOTH,
		FREQUENCY_CONVOLVE
	};

	FourierTransform(int transformHistorySize = 7, float cutOffLow = -1.0f, float cutOffHigh = -1.0f, float cutoffSmoothFrac = 0.0f);
	~FourierTransform();
	
	void init(Master* master, std::string name = "");
	void reInit();

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

	std::string getName() const { return _nameOfFT; }

	float getEnergy() { return _energyOfFt.getEnergy(); }
	History<float>* getEnergyHistory() { return _energyOfFt.getHistory(); }
private:
	FourierTransformHistory* _current;
	FourierTransformHistory* _next;

	FourierTransformHistory _working1;
	FourierTransformHistory _working2;

	Energy _energyOfFt;

	//state vars
	Master* _m;
	bool _initialised;
	bool _useSetters;
	int _historySize;
	std::string _nameOfFT;

	//low res output config
	int _maxLowResOutputSize;
	int _lowResOutputSize;
	float* _lowResOutputLogScale;

	//fourier transform descriptors vars
	int _numHarmonics;
	float _cutOffLow, _cutOffHigh;
	bool _minCutoffLow, _maxCutoffHigh;
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

	//calculates all the important frequency vars based of cutoffs
	void setFourierTransformFrequencyInfo();

	//called by init
	void initDefaultVars();

	//used when getting the cut fourier transform
	float smoothCutoff(int i);

	//setter functions
	void initSetters();
	void deleteSetters();
 };

