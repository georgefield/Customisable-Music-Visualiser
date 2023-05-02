#pragma once
#include "Master.h"
#include "Threshold.h"

#include "MFCCs.h"
#include "Energy.hpp"
#include "FourierTransform.h"
#include "SimilarityMatrixHandler.h"

#include "FilterBank.h"
#include "AudioManager.h"

#include <set>

class NoteOnset {
public:
	static enum DataExtractionAlg {
		ENERGY = 0,
		DER_OF_LOG_ENERGY = 1,
		HFC_DER_OF_LOG_ENERGY = 2,
		SPECTRAL_DISTANCE = 3,
		SPECTRAL_DISTANCE_WITH_PHASE = 4,
		COMBINATION_FAST = 6,
		COMBINATION = 7
	};

	NoteOnset(int historySize) :
		_rawOnsetDetectionHistory(historySize),
		_onsetPeaks(historySize),
		_outOnsetDetectionHistory(historySize),
		_displayPeaks(historySize),


		_thresholder(500), //~5 seconds

		_previousHFCenergy(0)
	{}

	~NoteOnset() {
		deleteSetters();
	}

	void init(Master* master, MFCCs* mfccs) {
		_m = master;

		_sampleLastCalculated = -1;

		initSetters();
	}

	void reInit() {
		_sampleLastCalculated = -1;

		_onsetPeaks.clear();
	}

	void calculateNext(
		DataExtractionAlg dataAlg,
		bool convolve
	);

	float getOnset() { return _outOnsetDetectionHistory.newest(); }
	History<float>* getOnsetHistory(bool convolved) { return &_outOnsetDetectionHistory; }

	History<Peak>* getPeakHistory() { return &_onsetPeaks; }
	History<float>* getDisplayPeaks() { return &_displayPeaks; }
private:
	Master* _m;

	int _sampleLastCalculated;
	
	float energy();
	float derivativeOfLogEnergy();
	float HFCderivativeOfLogEnergy();
	float _previousHFCenergy;
	float spectralDistance();
	float weightedPhaseDeviation();
	float combination();

	History<float> _rawOnsetDetectionHistory;
	History<float> _outOnsetDetectionHistory;

	Threshold _thresholder;

	History<float> _displayPeaks;
	History<Peak> _onsetPeaks; //in sample time

	void initSetters();
	void deleteSetters();
};