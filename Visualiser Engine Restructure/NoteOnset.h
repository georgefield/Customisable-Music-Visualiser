#pragma once
#include "Tools.h"
#include "Master.h"
#include "Threshold.h"

#include "Energy.hpp"
#include "FourierTransform.h"

#include "FilterBank.h"

#include <set>
#include "Limiter.h"

class NoteOnset {
public:
	static enum DataExtractionAlg {
		SPECTRAL_DISTANCE,
		DER_OF_LOG_ENERGY,
		BANDED_DER_OF_LOG_ENERGY
	};

	static enum PeakPickingAlg {
		CONVOLVE_THEN_THRESHOLD,
		THRESHOLD
	};

	NoteOnset(int historySize) :
		_onsetDetectionHistory(historySize),
		_onsetPeaks(historySize),
		_CONVonsetDetectionHistory(historySize),
		_displayPeaks(historySize),

		_generalLimiter(1.0, 0.5, 10.0, 0.2),

		_thresholder(2500), //approx 5 seconds

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

		_derOfLogEnergyBands.init(_m);
		_derOfLogEnergyBands.add(0.0, 150, 1.0f);
		_derOfLogEnergyBands.add(100, 600, 1.0f);
		_derOfLogEnergyBands.add(150, 1500, 1.0f);
		_derOfLogEnergyBands.add(600, 3000, 1.0f);
		_derOfLogEnergyBands.add(1500, 6000, 1.0f);
		_derOfLogEnergyBands.add(3000, 10000, 1.0f);
		_derOfLogEnergyBands.add(6000, 15000, 1.0f);
		_derOfLogEnergyBands.add(10000, 20000, 1.0f);
		_derOfLogEnergyBands.add(15000, 22050, 1.0f);
	}

	void reInit() {
		_sampleLastCalculated = -1;

		//thresholding vars
		_lastAboveThreshold = false;

		_ftForSpectralDistance.reInit();

		_derOfLogEnergyBands.reInit();

		_onsetPeaks.clear();
	}

	void calculateNext(
		DataExtractionAlg dataAlg = DataExtractionAlg::SPECTRAL_DISTANCE,
		PeakPickingAlg peakAlg = PeakPickingAlg::CONVOLVE_THEN_THRESHOLD
	);

	float getOnset() { return _onsetDetectionHistory.newest(); }
	History<float>* getOnsetHistory() { return &_onsetDetectionHistory; }
	History<float>* getCONVonsetHistory() { return &_CONVonsetDetectionHistory; }

	History<Peak>* getPeakHistory() { return &_onsetPeaks; }
	History<float>* getDisplayPeaks() { return &_displayPeaks; }
private:
	Master* _m;
	Energy* _energy;

	FourierTransform _ftForSpectralDistance;

	int _sampleLastCalculated;
	
	float derivativeOfLogEnergy();
	float spectralDistanceOfHarmonics();

	float bandedDerOfLogEnergy();
	FilterBank _derOfLogEnergyBands;

	History<float> _onsetDetectionHistory;
	History<float> _CONVonsetDetectionHistory;

	Limiter _generalLimiter;
	Threshold _thresholder;

	History<float> _displayPeaks;
	History<Peak> _onsetPeaks; //in sample time
	bool _lastAboveThreshold;

	void initSetters();
	void deleteSetters();
};