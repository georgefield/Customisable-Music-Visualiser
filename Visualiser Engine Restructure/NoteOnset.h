#pragma once
#include "Tools.h"
#include "Master.h"
#include "Threshold.h"

#include "MFCCs.h"
#include "Energy.hpp"
#include "FourierTransform.h"
#include "SimilarityMatrixHandler.h"

#include "FilterBank.h"

#include <set>
#include "Limiter.h"

class NoteOnset {
public:
	static enum DataExtractionAlg {
		ENERGY = 0,
		DER_OF_LOG_ENERGY = 1,
		HFC_DER_OF_LOG_ENERGY = 2,
		SPECTRAL_DISTANCE = 3,
		SPECTRAL_DISTANCE_WITH_PHASE = 4,
		SIM_MATRIX_MEL_SPEC = 5,
		COMBINATION_FAST = 6,
		COMBINATION = 7
	};

	NoteOnset(int historySize) :
		_onsetDetectionHistory(historySize),
		_onsetPeaks(historySize),
		_CONVonsetDetectionHistory(historySize),
		_displayPeaks(historySize),


		_thresholder(1000), //approx 10 seconds

		_ftForSpectralDistance(2),
		_simMatrix(true),

		_previousHFCenergy(0)
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

		//init spectral distance fourier transform with smoothing features--
		_ftForSpectralDistance.init(_m, -1);
		_ftForSpectralDistance.setSmoothVars(0.1, 0.3, 0.5);
		_ftForSpectralDistance.setSmoothEffect(0, FourierTransform::SMOOTH);
		_ftForSpectralDistance.setFrequencyConvolvingVars(5);
		_ftForSpectralDistance.setSmoothEffect(1, FourierTransform::FREQUENCY_CONVOLVE);
		//--

		_simMatrix._SMinfo._matrixSize = 2000 / SP::vars._desiredCPS;
		_simMatrix._SMinfo._useFuture = true;
		_simMatrix.init(_m);
		_simMatrix.linkToMelSpectrogram();
	}

	void reInit() {
		_sampleLastCalculated = -1;

		_onsetPeaks.clear();

		//thresholding vars
		_lastAboveThreshold = false;

		_ftForSpectralDistance.reInit();

		_simMatrix._SMinfo._matrixSize = 2000 / SP::vars._desiredCPS;
		_simMatrix._SMinfo._useFuture = true;
		_simMatrix.reInit();
		_simMatrix.linkToMelSpectrogram();
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

	SimilarityMatrixHandler _simMatrix;

	FourierTransform _ftForSpectralDistance;

	int _sampleLastCalculated;
	
	float energy();
	float derivativeOfLogEnergy();
	float HFCderivativeOfLogEnergy();
	float _previousHFCenergy;
	float spectralDistance();
	float spectralDistanceWithPhase();
	float similarityMatrixMelSpectrogram();
	float combinationFast();
	float combination();

	History<float> _onsetDetectionHistory;
	History<float> _CONVonsetDetectionHistory;

	Threshold _thresholder;

	History<float> _displayPeaks;
	History<Peak> _onsetPeaks; //in sample time
	bool _lastAboveThreshold;

	void initSetters();
	void deleteSetters();
};