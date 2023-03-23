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

class NoteOnset {
public:
	static enum DataExtractionAlg {
		ENERGY = 0,
		DER_OF_LOG_ENERGY = 1,
		HFC_DER_OF_LOG_ENERGY = 2,
		SPECTRAL_DISTANCE = 3,
		SPECTRAL_DISTANCE_WITH_PHASE = 4,
		SIM_MATRIX_MFCC = 5,
		COMBINATION_FAST = 6,
		COMBINATION = 7
	};

	NoteOnset(int historySize) :
		_rawOnsetDetectionHistory(historySize),
		_onsetPeaks(historySize),
		_outOnsetDetectionHistory(historySize),
		_displayPeaks(historySize),


		_thresholder(1000), //~10 seconds

		_ftForSpectralDistance(2),
		_simMatrix(false), //dont use setters

		_previousHFCenergy(0)
	{}

	~NoteOnset() {
		deleteSetters();
	}

	void init(Master* master, MFCCs* mfccs) {
		_m = master;
		_mfcc = mfccs;

		_sampleLastCalculated = -1;

		initSetters();

		//init spectral distance fourier transform with smoothing features--
		_ftForSpectralDistance.init(_m, -1);
		_ftForSpectralDistance.setSmoothVars(0.1, 0.3, 0.5);
		_ftForSpectralDistance.setSmoothEffect(0, FourierTransform::SMOOTH);
		_ftForSpectralDistance.setFrequencyConvolvingVars(5);
		_ftForSpectralDistance.setSmoothEffect(1, FourierTransform::FREQUENCY_CONVOLVE);
		//--

		_simMatrix._SMinfo._matrixSize = 0.15 * SP::vars._desiredCPS;
		_simMatrix._SMinfo._useFuture = true;
		_simMatrix._SMinfo._contrastFactor = 10.0f;
		_simMatrix._SMinfo._measureType = PERCUSSION;
		_simMatrix.init(_m);
		_simMatrix.linkToMFCCs(1, SP::consts._numMelBands);
	}

	void reInit() {
		_sampleLastCalculated = -1;

		_onsetPeaks.clear();

		_ftForSpectralDistance.reInit();

		_simMatrix.reInit();
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
	MFCCs* _mfcc;

	SimilarityMatrixHandler _simMatrix;

	FourierTransform _ftForSpectralDistance;

	int _sampleLastCalculated;
	
	float energy();
	float derivativeOfLogEnergy();
	float HFCderivativeOfLogEnergy();
	float _previousHFCenergy;
	float spectralDistance();
	float weightedPhaseDeviation();
	float similarityMatrixMFCC();
	float combinationFast();
	float combination();

	History<float> _rawOnsetDetectionHistory;
	History<float> _outOnsetDetectionHistory;

	Threshold _thresholder;

	History<float> _displayPeaks;
	History<Peak> _onsetPeaks; //in sample time

	void initSetters();
	void deleteSetters();
};