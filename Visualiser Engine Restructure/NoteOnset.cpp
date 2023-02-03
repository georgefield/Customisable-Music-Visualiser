#include "NoteOnset.h"


void NoteOnset::calculateNext(DataExtractionAlgorithm dataAlg, PeakPickingAlgorithm peakAlg) {

	//make sure not called twice on same frame
	if (_sampleLastCalculated == _m->_currentSample) {
		return;
	}
	_sampleLastCalculated = _m->_currentSample;

	//onset detection
	float onsetValue = 0;
	if (dataAlg == DataExtractionAlgorithm::DER_OF_LOG_ENERGY) {
		_energy->calculateNext(LINEAR_PYRAMID); //depends on energy
		onsetValue = derivativeOfLogEnergy();
	}
	if (dataAlg == DataExtractionAlgorithm::SPECTRAL_DISTANCE) {
		_m->calculateFft(); //depends on fft for spectral information
		onsetValue = spectralDistanceOfHarmonics();
	}
	if (dataAlg == DataExtractionAlgorithm::SPECTRAL_DISTANCE_CONVOLVED_HARMONICS) {
		_m->calculateTimeConvolvedFft(); //depends on time convolved fft
		onsetValue = spectralDistanceOfTimeConvolvedHarmonics();
	}
	float limitedValue = _generalLimiter.clampMedianThenLimit(onsetValue);
	_onsetDetectionHistory.add(limitedValue, _m->_currentSample);

	_CONVonsetDetectionHistory.add(
		_m->sumOfConvolutionOfHistory(&_onsetDetectionHistory, 5, LINEAR_PYRAMID),
		_m->_currentSample
	);

	//peak detection
	bool isPeak = false;
	if (peakAlg == PeakPickingAlgorithm::THRESHOLD) {
		isPeak = thresholdPercent(getOnsetHistory(), 10); //peak if in top 10%
	}
	if (peakAlg == PeakPickingAlgorithm::CONVOLVE_THEN_THRESHOLD) {
		isPeak = thresholdPercent(getCONVonsetHistory(), 10); //peak if in top 10%
	}
	if (isPeak) {
		_onsetPeaks.add(_m->_currentSample);
	}
}

//*** onset algorithms ***

float NoteOnset::derivativeOfLogEnergy() {

	float one_over_dt = ((float)_m->_sampleRate / (float)(_m->_currentSample - _m->_previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//--change of energy per second, using log scale as human ear is log scale
	//good for detecting beats of music with 4 to floor drum pattern

	float derOfLogEnergy = (logf(_energy->getEnergy()) - logf(_energy->getHistory()->previous())); // d(log(E))
	derOfLogEnergy *= one_over_dt; // *= 1/dt
	if (isnan(derOfLogEnergy) || isinf(derOfLogEnergy)) { derOfLogEnergy = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf

	return (derOfLogEnergy < 0 ? 0 : derOfLogEnergy); //only take onset (positive change in energy)

	//--
}

float NoteOnset::spectralDistanceOfHarmonics() {

	float one_over_dt = ((float)_m->_sampleRate / (float)(_m->_currentSample - _m->_previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//--spectral distance, take fourier transform as N dimension point, calculates L2norm from previous transform to current
	float spectralDistance = Tools::L2normIncreasingDimensionsOnly(
		_m->_fftOutput->newest(),
		_m->_fftOutput->previous(),
		_m->getNumHarmonics()
	);

	spectralDistance *= one_over_dt; //must be scaled for time

	return spectralDistance;
	//--
}

float NoteOnset::spectralDistanceOfTimeConvolvedHarmonics() {

	float one_over_dt = ((float)_m->_sampleRate / (float)(_m->_currentSample - _m->_previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//same as above but with the convolved harmonics--
	float spectralDistanceConvolvedHarmonics = Tools::L2normIncreasingDimensionsOnly(
		_m->_timeConvolvedFftOutput.newest(),
		_m->_timeConvolvedFftOutput.previous(),
		_m->getNumHarmonics()
	);
	spectralDistanceConvolvedHarmonics *= one_over_dt;

	return spectralDistanceConvolvedHarmonics;
	//--
}

//*** peak picking algorithms ***

bool NoteOnset::thresholdPercent(History<float>* historyToUse, float topXpercent)
{
	_thresholder.addValue(historyToUse->newest());
	return _thresholder.testThreshold(historyToUse->newest(), topXpercent);
}
