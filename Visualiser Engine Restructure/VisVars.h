#pragma once

//variables that define settings of the visualiser, saved
struct VisVariables {
	//which to compute--
	bool _computeFrequencyBands = true;
	bool _computeNoteOnset = true;
	bool _computeTempoDetection = true;
	bool _computeMFCCs = true;
	bool _computeSimilarityMatrix = true;
	//--

	//modifyable properties of algorithms-

	//note onset
	int _onsetDetectionFunctionEnum = 0;
	bool _convolveOnsetDetection = true;
	int _convolveWindowSize = 7;
	float _thresholdPercentForPeak = 7.5;

	float _detectionFunctionGain = 1.0f;
	float _detectionFunctionCompressionThreshold = 1.0f;
	float _detectionFunctionCompressionRatio = 1.0f;
	bool _clampBetween0and1 = false;

	//note onset combination vars
	float _energyMixAmount = 1;
	float _doleMixAmount = 0;
	float _HFCdoleMixAmount = 0;
	float _sdMixAmount = 0;
	float _wpdMixAmount = 0;
	float _leftoverConstantFactor = 0;
	//

	//tempo detection
	float MIN_TEMPO = 80;
	float MAX_TEMPO = 200;

	//similarity matrix values that can be changed without needing a reinit
	bool _fastSimilarityMatrixTexture = false; //always false
	bool _computeTexture = true;
	float _similarityMatrixTextureContrastFactor = 20.0f;
	int _matrixMeasureEnum = 0;
	//--

	//other properties--
	float _desiredCPS = 100; //desired calculations per second
	float _masterFTgain = 8.0f;
	float _clearColour[3] = { 0.1, 0.1, 0.1 };
	//--
};

//constant values
const struct VisConstants {
	//consts
	//resolution options--
	const int defaultSW = 1024;
	const int defaultSH = 768;
	//--

	const int _generalHistorySize = 1000;


	const int _STFTsamples = 4096;
	const int _maxFourierTransforms = 4;
	const int _numMelBands = 25;
	const int _peakAmplitudeWindowSizeInSamples = 4096;

	//loopback lag--
	const int _numLagsBeforeReducingCPS = 4;
	const float _timeWindowForLag = 3.0f; //lag 4 times in 3 seconds then reduce CPS
	//--
	//audio file lag--
	const float _minTimeNextCalculationSampleBehind = 1.0f;
	const int _minAmountNextCalculationSampleBehind = 2000; //if behind by more than 2000 samples and behind for more than 1 second then reduce CPS
	//--

	const float _CPSreduceFactor = 0.90;

	//loopback cache and safety
	const int _requiredLoopbackCacheLength = 8192; //usually does 441/480 samples at a time , 10x for safety
	const int _loopbackCacheSafetyBuffer = 2000; //~0.1 seconds, delay for this long after recieving loopback samples
	const int _finalLoopbackStorageSize = 4096;

	const int _availiableScriptingVars = 3;
};

//global vars used in different locations in program
struct VisGlobal {

	bool _wasCPSautoDecreased = false;
	bool _wasSignalProcessingReset = false;
	float _calculationFrameTime = 0.01;
	bool _isCalculationFrame = false;
};

struct Vis {
	static VisVariables vars;
	static VisConstants consts;
	static VisGlobal comms;
};