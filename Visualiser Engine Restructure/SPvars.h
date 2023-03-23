#pragma once

struct SPvarsStruct {
	//which to compute--
	bool _computeFourierTransform = true;
	bool _computeNoteOnset = true;
	bool _computeTempoDetection = true;
	bool _computeMFCCs = true;
	bool _computeSimilarityMatrix = true;
	//--

	//modifyable properties of algorithms--
	//general
	bool _wasCPSautoDecreased = false;
	bool _wasSignalProcessingReset = false;

	//note onset
	int _onsetDetectionFunctionEnum = 6;
	bool _convolveOnsetDetection = true;
	int _convolveWindowSize = 7;
	float _thresholdPercentForPeak = 7.5;

	float _detectionFunctionGain = 1.0f;
	float _detectionFunctionCompressionThreshold = 1.0f;
	float _detectionFunctionCompressionRatio = 1.0f;
	bool _clampBetween0and1 = false;

	//tempo detection
	float MIN_TEMPO = 40;
	float MAX_TEMPO = 230;

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

const struct SPconstsStruct {
	//consts
	const int _generalHistorySize = 1000;

	const int _numMelBands = 25;

	const int _STFTsamples = 4096;

	const float _lagTimeBeforeReducingCPS = 1.0f;

	const float _CPSreduceFactor = 0.75;
	
	const int _maxFourierTransforms = 4;
};

struct SP {
	static SPconstsStruct consts;
	static SPvarsStruct vars;
};