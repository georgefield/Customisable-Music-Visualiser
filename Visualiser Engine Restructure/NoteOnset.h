#pragma once
#include "Tools.h"
#include "Master.h"
#include "Threshold.h"

#include "Energy.hpp"
#include "FFTs.h"

#include <set>
#include "Limiter.h"

struct Peak {
	int onset;
	float salience;
};

class NoteOnset {
public:
	enum class DataExtractionAlgorithm {
		SPECTRAL_DISTANCE,
		SPECTRAL_DISTANCE_CONVOLVED_HARMONICS,
		DER_OF_LOG_ENERGY
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
		_lastAboveThresh(false),
		_goingDownFromPeak(false),
		_thresholder(500)
	{}

	void init(Master* master, Energy* energy, FFTs* FFTs) {
		_m = master;
		_energy = energy;
		_FFTs = FFTs;

		_sampleLastCalculated = -1;
		//thresholding vars
		_justGoneOverThreshold = false;

		_currentConvolvedHarmonics = new float[_FFTs->_numHarmonics];
		_previousConvolvedHarmonics = new float[_FFTs->_numHarmonics];
	}

	void setFourierTransformToUseForSpectralDistance(FourierTransformType type) {
		_fourierTransformTypeForSpectralDistance = type;
	}

	void calculateNext(
		DataExtractionAlgorithm dataAlg = DataExtractionAlgorithm::SPECTRAL_DISTANCE_CONVOLVED_HARMONICS,
		PeakPickingAlgorithm peakAlg = PeakPickingAlgorithm::THRESHOLD
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
	FFTs* _FFTs;

	FourierTransformType _fourierTransformTypeForSpectralDistance;
	int _sampleLastCalculated;
	
	float derivativeOfLogEnergy();
	float spectralDistanceOfHarmonics(History<float*>* fftHistoryToUse);
	float spectralDistanceOfConvolvedHarmonics(History<float*>* fftHistoryToUse);

	bool thresholdPercent(History<float>* historyToUse, float topXpercent);

	History<float> _onsetDetectionHistory;
	History<float> _CONVonsetDetectionHistory;

	Limiter _generalLimiter;
	Threshold _thresholder;
	bool _lastAboveThresh;
	bool _goingDownFromPeak;

	History<Peak> _onsetPeaks; //in sample time
	bool _justGoneOverThreshold;

	float* _currentConvolvedHarmonics;
	float* _previousConvolvedHarmonics;
};