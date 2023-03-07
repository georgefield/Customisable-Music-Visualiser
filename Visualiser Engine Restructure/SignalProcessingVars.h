#pragma once
struct SPvars {
	struct UI {	//Can interact with them through UI
		//which to compute--
		static bool _computeFourierTransform;
		static bool _computeRms;
		static bool _computeNoteOnset;
		static bool _computeTempoDetection;
		static bool _computeMFCCs;
		static bool _computeSimilarityMatrix;
		//--

		//modifyable properties of algorithms--
		//note onset
		static int _onsetDetectionFunctionEnum;
		static bool _convolveOnsetDetection;
		static int _convolveWindowSize;
		static float _thresholdPercentForPeak;

		//tempo detection
		static float MIN_TEMPO;
		static float MAX_TEMPO;

		//similarity matrix
		static int _nextSimilarityMatrixSize;
		static bool _fastSimilarityMatrixTexture;
		static float _similarityMatrixTextureContrastFactor;
		static int _matrixMeasureEnum;
		//--
	};

	struct Const {
		static const int _generalHistorySize;

		static const int _numMelBands;

	};
};