#pragma once
#include "Tools.h"
#include "Master.h"
#include "Threshold.h"

#include "Energy.hpp"

#include <set>
#include "Limiter.h"

class NoteOnset {
public:
	enum class DataExtractionAlgorithm {
		SPECTRAL_DISTANCE,
		DER_OF_LOG_ENERGY,
		SPECTRAL_DISTANCE_CONVOLVED_HARMONICS
	};

	enum class PeakPickingAlgorithm {
		CONVOLVE_THEN_THRESHOLD,
		THRESHOLD
	};

	NoteOnset(int historySize) :
		_onsetDetectionHistory(historySize),
		_onsetPeaks(historySize),
		_CONVonsetDetectionHistory(historySize),
		_generalLimiter(1.0, 0.5, 10.0, 0.2),
		_thresholder(500)
	{}

	void init(Master* master, Energy* energy) {
		_m = master;
		_energy = energy;
		_sampleLastCalculated = -1;
		//thresholding vars
		_justGoneOverThreshold = false;
	}

	void calculateNext(
		DataExtractionAlgorithm dataAlg = DataExtractionAlgorithm::SPECTRAL_DISTANCE_CONVOLVED_HARMONICS, 
		PeakPickingAlgorithm peakAlg = PeakPickingAlgorithm::THRESHOLD
	);

	History<float>* getOnsetHistory() {
		return &_onsetDetectionHistory;
	}

	History<int>* getPeakHistory() {
		return &_onsetPeaks;
	}

	History<float>* getCONVonsetHistory() {
		return &_CONVonsetDetectionHistory;
	}
private:
	Master* _m;
	Energy* _energy;

	int _sampleLastCalculated;

	float derivativeOfLogEnergy();
	float spectralDistanceOfHarmonics();
	float spectralDistanceOfTimeConvolvedHarmonics();

	bool thresholdPercent(History<float>* historyToUse, float topXpercent);

	History<float> _onsetDetectionHistory;
	History<float> _CONVonsetDetectionHistory;

	Limiter _generalLimiter;
	Threshold _thresholder;

	History<int> _onsetPeaks; //in sample time
	bool _justGoneOverThreshold;
};