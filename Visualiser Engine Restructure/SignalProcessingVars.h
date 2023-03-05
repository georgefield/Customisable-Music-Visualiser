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
		static bool _computeFutureSimilarityMatrix;
		//--

		//modifyable properties of algorithms--
		static int _nextSimilarityMatrixSize;
		//--
	};

	struct Const {
		static const int _generalHistorySize;

		static const int _numMelBands;
	};
};