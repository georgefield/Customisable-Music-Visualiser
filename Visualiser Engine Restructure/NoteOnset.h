#pragma once
#include "Tools.h"
#include "Master.h"
#include "Threshold.h"

#include "MFCCs.h"
#include "Energy.hpp"
#include "FourierTransform.h"
#include "SelfSimilarityMatrix.h"

#include "FilterBank.h"

#include <set>
#include "Limiter.h"

class NoteOnset {
public:
	static enum DataExtractionAlg {
		ENERGY = 0,
		DER_OF_LOG_ENERGY = 1,
		BANDED_DER_OF_LOG_ENERGY = 2,
		SPECTRAL_DISTANCE = 3,
		SPECTRAL_DISTANCE_HFC_WEIGHTED = 4,
		SIM_MATRIX_MEL_SPEC = 5,
		COMBINATION = 6
	};

	NoteOnset(int historySize) :
		_onsetDetectionHistory(historySize),
		_onsetPeaks(historySize),
		_CONVonsetDetectionHistory(historySize),
		_displayPeaks(historySize),


		_thresholder(1000), //approx 10 seconds

		_ftForSpectralDistance(2),
		_simMatrix(historySize),

		_energy(2)
	{}

	~NoteOnset() {
		deleteSetters();
	}

	void init(Master* master, MFCCs* mfccs) {
		_m = master;
		_mfcc = mfccs;

		_sampleLastCalculated = -1;

		//thresholding vars		
		_lastAboveThreshold = false;

		initSetters();

		//init onset detection function class dependencies
		_energy.init(_m, "");

		_ftForSpectralDistance.init(_m, "");

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

		_simMatrix.init(2000 / SPvars::UI::_desiredCPS);
		_simMatrix.linkToMelSpectrogram(_mfcc);
	}

	void reInit() {
		_sampleLastCalculated = -1;

		_onsetPeaks.clear();

		//thresholding vars
		_lastAboveThreshold = false;

		//reinit 
		_energy.reInit();

		_ftForSpectralDistance.reInit();

		_derOfLogEnergyBands.reInit();

		_simMatrix.reInit(2000 / SPvars::UI::_desiredCPS);
		_simMatrix.linkToMelSpectrogram(_mfcc);
	}

	void calculateNext(
		DataExtractionAlg dataAlg,
		bool convolve
	);

	float getOnset() { return _onsetDetectionHistory.newest(); }
	History<float>* getOnsetHistory(bool convolved) {
		if (convolved) return &_CONVonsetDetectionHistory;
		else return &_onsetDetectionHistory;
	}

	History<Peak>* getPeakHistory() { return &_onsetPeaks; }
	History<float>* getDisplayPeaks() { return &_displayPeaks; }
private:
	Master* _m;
	MFCCs* _mfcc;
	Energy _energy;

	SelfSimilarityMatrix _simMatrix;

	FourierTransform _ftForSpectralDistance;

	int _sampleLastCalculated;
	
	float energy();
	float derivativeOfLogEnergy();
	float bandedDerOfLogEnergy();
	float spectralDistanceOfHarmonics(bool HFC);
	float similarityMatrixMelSpectrogram();
	float combination();

	FilterBank _derOfLogEnergyBands;

	History<float> _onsetDetectionHistory;
	History<float> _CONVonsetDetectionHistory;

	Threshold _thresholder;

	History<float> _displayPeaks;
	History<Peak> _onsetPeaks; //in sample time
	bool _lastAboveThreshold;

	void initSetters();
	void deleteSetters();
};