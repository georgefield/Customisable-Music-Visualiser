#pragma once
#include "Tools.h"
#include "Master.h"
#include "Threshold.h"

#include "Energy.hpp"
#include "FourierTransform.h"

#include <set>
#include "Limiter.h"

struct Peak {
	int onset;
	float salience;
};

class NoteOnset {
public:
	static enum DataExtractionAlg {
		SPECTRAL_DISTANCE,
		DER_OF_LOG_ENERGY
	};

	static enum PeakPickingAlg {
		CONVOLVE_THEN_THRESHOLD,
		THRESHOLD
	};

	NoteOnset(int historySize) :
		_onsetDetectionHistory(historySize),
		_onsetPeaks(historySize),
		_CONVonsetDetectionHistory(historySize),

		_generalLimiter(1.0, 0.5, 10.0, 0.2),
		_lastAboveThresh(false),
		_goingDownFromPeak(false),

		_thresholder(500),

		_ftForSpectralDistance(2)
	{}

	FourierTransform _ftForSpectralDistance;

	void init(Master* master, Energy* energy) {
		_m = master;
		_energy = energy;

		_sampleLastCalculated = -1;

		//thresholding vars
		_justGoneOverThreshold = false;

		_ftForSpectralDistance.init(_m);
	}

	void calculateNext(
		DataExtractionAlg dataAlg = DataExtractionAlg::SPECTRAL_DISTANCE,
		PeakPickingAlg peakAlg = PeakPickingAlg::CONVOLVE_THEN_THRESHOLD
	);

	History<float>* getOnsetHistory() {
		return &_onsetDetectionHistory;
	}

	History<Peak>* getPeakHistory() {
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

	bool thresholdPercent(History<float>* historyToUse, float topXpercent);

	History<float> _onsetDetectionHistory;
	History<float> _CONVonsetDetectionHistory;

	Limiter _generalLimiter;
	Threshold _thresholder;
	bool _lastAboveThresh;
	bool _goingDownFromPeak;

	History<Peak> _onsetPeaks; //in sample time
	bool _justGoneOverThreshold;
};