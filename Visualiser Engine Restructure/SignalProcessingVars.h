#pragma once
struct SPvarsStruct {
	//which to compute--
	bool _computeFourierTransform = true;
	bool _computeRms = true;
	bool _computeNoteOnset = true;
	bool _computeTempoDetection = true;
	bool _computeMFCCs = true;
	bool _computeSimilarityMatrix = true;
	//--

	//modifyable properties of algorithms--
	//general
	bool _wasCPSautoDecreased = false;
	bool _wasSignalProcessingReset = false;
	int _nextCalculationSample = 0;

	//note onset
	int _onsetDetectionFunctionEnum = 6;
	bool _convolveOnsetDetection = true;
	int _convolveWindowSize = 7;
	float _thresholdPercentForPeak = 7.5;

	//tempo detection
	float MIN_TEMPO = 40;
	float MAX_TEMPO = 230;

	//similarity matrix
	int _nextSimilarityMatrixSize = 100;
	bool _fastSimilarityMatrixTexture = true;
	float _similarityMatrixTextureContrastFactor = 20.0f;
	int _matrixMeasureEnum = 0;
	bool _useFutureSimilarityMatrix = false;
	//--

	//other properties--
	float _desiredCPS = 100; //desired calculations per second
	float _masterFTgain = 8.0f;
	//--

	//consts
	const int _generalHistorySize = 1000;

	const int _numMelBands = 25;

	const int _STFTsamples = 4096;

	const float _lagTimeBeforeReducingCPS = 1.0f;

	const float _CPSreduceFactor = 0.75;
};

static SPvarsStruct SPvars;