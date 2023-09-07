#pragma once
#include "Master.h"
#include "FourierTransformHistory.h"
#include "Energy.hpp"
#include "VisVars.h"

class FourierTransform {
public:
	static enum FunctionType {
		FREQUENCY_CONVOLVE = 0,
		SMOOTH = 1,
		TIME_CONVOLVE = 2,
		NO_FUNCTION = 3
	};

	struct FTinfo {
		int id = -1;

		int historySize = 7;
		float cutoffLow = -1;
		bool isMinCutoffLow = true;
		float cutoffHigh = -1;
		bool isMaxCutoffHigh = true;
		float cutoffSmoothFrac = 0.0f;
	
		FunctionType applyFirst = NO_FUNCTION;
		FunctionType applySecond = NO_FUNCTION;
		FunctionType applyThird = NO_FUNCTION;

		//freq convolving vars
		int freqWindowSize = 5;
		//smooth vars
		float attack = 0.15;
		float release = 0.5;
		float maxAccelerationPerSecond = 0.5;
		//time convolving
		int timeWindowSize = 7;
	};

	FourierTransform(int transformHistorySize = 7, float cutOffLow = -1.0f, float cutOffHigh = -1.0f, float cutoffSmoothFrac = 0.0f);
	~FourierTransform();
	
	void init(Master* master, int FTid = -1);
	void reInit();

	void calculateNext();

	//set what filters to apply--
	void setSmoothEffect(int firstSecondOrThird, FunctionType function);
	void addSmoothEffect(FunctionType function);
	void removeSmoothEffect();
	bool canAddSmoothEffect() { return _FTinfo.applyThird == NO_FUNCTION; }
	bool canRemoveSmoothEffect() { return _FTinfo.applyFirst != NO_FUNCTION; }
	//--

	//for setting functions for ft vars--
	void setFrequencyConvolvingVars(int windowSize) {
		_FTinfo.freqWindowSize = windowSize;
	}

	void setSmoothVars(float attack, float release, float maxAccelerationPerSecond) {
		_FTinfo.attack = attack; _FTinfo.release = release; _FTinfo.maxAccelerationPerSecond = maxAccelerationPerSecond;
	}

	void setTimeConvolvingVars(int previousXtransform) {
		_FTinfo.timeWindowSize = previousXtransform;
	}
	//--

	//getters
	FourierTransformHistory* getHistory() { return _current; }
	float* getOutput() const { return _current->newest(); }
	int getNumHarmonics() const { return _current->numHarmonics(); }

	float* getLowResOutput();
	int getLowResOutputSize() const { return _lowResOutputSize; }

	Master* getMasterPtr() const { return _m; }

	int getId() const { return _FTinfo.id; }

	float getEnergy() { return _energyOfFt.getEnergy(); }
	History<float>* getEnergyHistory() { return _energyOfFt.getHistory(); }

	FTinfo _FTinfo;
private:
	FourierTransformHistory* _current;
	FourierTransformHistory* _next;

	FourierTransformHistory _working1;
	FourierTransformHistory _working2;

	Energy _energyOfFt;

	//parts of the calculate next function
	void beginCalculation(); //applying filters must be between begin and end
	void endCalculation();

	void applyFunction(FunctionType type); //must be called in every frame to work
	void applyFunctions(FunctionType* args, int numArgs);

	//state vars (not needed in FT info)
	Master* _m;
	bool _initialised;
	bool _useSetters;

	//low res output config
	int _lowResOutputSize; //min 50 & num harmonics
	float* _lowResOutputLogScale;
	bool _lowResOutputCalculatedThisFrame;
	void initLowResOutput();
	void calculateLowResOutput();

	//fourier transform descriptors vars
	int _numHarmonics;
	int _harmonicLow, _harmonicHigh;

	//smooth arrays
	float* _smoothedFt; //store seperately as need to remember smoothed array how is was before another filter applied maybe
	float* _smoothedFtDot;

	//apply functions to ft--
	//defined in .cpp
	void applySmoothing(float* in, float* current);
	void applyTimeConvolving(FourierTransformHistory* in, float* out);
	void applyFreqConvolving(float* in, float* out);
	//--

	//calculates all the important frequency vars based of cutoffs
	void setFourierTransformFrequencyInfo();

	//used when getting the cut fourier transform
	float smoothCutoff(int i);

	//setter functions
	void setUpdaters();
	void removeUpdaters();
 };

