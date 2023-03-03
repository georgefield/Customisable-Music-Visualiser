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

		_thresholder(500),

		_ftForSpectralDistance(2)
	{}

	~NoteOnset() {
		deleteSetters();
	}

	void init(Master* master) {
		_m = master;

		_sampleLastCalculated = -1;

		//thresholding vars		
		_lastAboveThreshold = false;

		_ftForSpectralDistance.init(_m, "Note Onset FT", false);

		initSetters();
	}

	void reInit() {
		_sampleLastCalculated = -1;

		//thresholding vars
		_lastAboveThreshold = false;

		_ftForSpectralDistance.reInit();
	}

	void calculateNext(
		DataExtractionAlg dataAlg = DataExtractionAlg::SPECTRAL_DISTANCE,
		PeakPickingAlg peakAlg = PeakPickingAlg::CONVOLVE_THEN_THRESHOLD
	);

	float getOnset() {
		return _onsetDetectionHistory.newest();
	}

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

	FourierTransform _ftForSpectralDistance;

	int _sampleLastCalculated;
	
	float derivativeOfLogEnergy();
	float spectralDistanceOfHarmonics();

	bool thresholdPercent(History<float>* historyToUse, float topXpercent);

	History<float> _onsetDetectionHistory;
	History<float> _CONVonsetDetectionHistory;

	Limiter _generalLimiter;
	Threshold _thresholder;

	History<Peak> _onsetPeaks; //in sample time
	bool _lastAboveThreshold;

	void initSetters();
	void deleteSetters();
};