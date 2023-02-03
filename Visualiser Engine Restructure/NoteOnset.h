#pragma once
#include "Tools.h"
#include "Master.h"

#include "Energy.hpp"

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
		_CONVonsetDetectionHistory(historySize)
	{}

	void init(Master* master, Energy* energy) {
		_m = master;
		_energy = energy;
		_sampleLastCalculated = -1;
	}

	void calculateNext(DataExtractionAlgorithm dataAlg, PeakPickingAlgorithm peakAlg);

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

	bool threshold(float value, float thresh);

	History<float> _onsetDetectionHistory;
	History<float> _CONVonsetDetectionHistory;

	History<int> _onsetPeaks; //in sample time
};