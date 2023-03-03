#include "NoteOnset.h"


void NoteOnset::calculateNext(DataExtractionAlg dataAlg, PeakPickingAlg peakAlg) {

	//make sure not called twice on same frame
	if (_sampleLastCalculated == _m->_currentSample) {
		return;
	}
	_sampleLastCalculated = _m->_currentSample;

	//onset detection
	float onsetValue = 0;
	if (dataAlg == DataExtractionAlg::DER_OF_LOG_ENERGY) {
		_energy->calculateNext(_m->_fftHistory.newest(), _m->_fftHistory.numHarmonics()); //depends on energy
		onsetValue = derivativeOfLogEnergy();
		onsetValue *= 10; //somewhat normalise
	}
	if (dataAlg == DataExtractionAlg::SPECTRAL_DISTANCE) {
		_ftForSpectralDistance.beginCalculation();
		_ftForSpectralDistance.applyFunction(FourierTransform::SMOOTH);
		_ftForSpectralDistance.applyFunction(FourierTransform::FREQUENCY_CONVOLVE);
		_ftForSpectralDistance.endCalculation();
		onsetValue = 20*spectralDistanceOfHarmonics();
	}

	_onsetDetectionHistory.add(onsetValue, _m->_currentSample);

	_CONVonsetDetectionHistory.add(
		_m->sumOfConvolutionOfHistory(&_onsetDetectionHistory, 15, LINEAR_PYRAMID),
		_m->_currentSample
	);

	//peak detection
	bool aboveThreshold = false;
	if (peakAlg == PeakPickingAlg::THRESHOLD) {
		aboveThreshold = thresholdPercent(getOnsetHistory(), 10); //peak if in top 7%
	}
	if (peakAlg == PeakPickingAlg::CONVOLVE_THEN_THRESHOLD) {
		aboveThreshold = thresholdPercent(getCONVonsetHistory(), 10); //peak if in top 7%
	}

	if (aboveThreshold && !_lastAboveThreshold) {
		_lastAboveThreshold = true;
		_onsetPeaks.add({ _m->_currentSample, onsetValue });
	}
	if (!aboveThreshold) {
		_lastAboveThreshold = false;
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

	//same as above but with the convolved harmonics--
	float spectralDistanceConvolvedHarmonics = Tools::L2distanceMetricIncDimOnly(
		_ftForSpectralDistance.getHistory()->newest(),
		_ftForSpectralDistance.getHistory()->previous(),
		_ftForSpectralDistance.getHistory()->numHarmonics()
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

void NoteOnset::initSetters()
{
	VisualiserShaderManager::addHistoryAsPossibleSSBOsetter("Note Onset", &_onsetDetectionHistory);
}

void NoteOnset::deleteSetters()
{
	VisualiserShaderManager::deleteHistoryAsPossibleSSBOsetter("Note Onset");
}
