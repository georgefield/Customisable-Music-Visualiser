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
		onsetValue = derivativeOfLogEnergy();
	}
	if (dataAlg == BANDED_DER_OF_LOG_ENERGY) {
		onsetValue = bandedDerOfLogEnergy();
	}
	if (dataAlg == DataExtractionAlg::SPECTRAL_DISTANCE) {
		onsetValue = spectralDistanceOfHarmonics();
	}
	if (dataAlg == DataExtractionAlg::SIM_MATRIX) {
		onsetValue = similarityMatrixMelSpectrogram();
	}

	_onsetDetectionHistory.add(onsetValue, _m->_currentSample);

	_CONVonsetDetectionHistory.add(
		_m->sumOfConvolutionOfHistory(&_onsetDetectionHistory, 15, LINEAR_PYRAMID),
		_m->_currentSample
	);

	//peak detection
	if (peakAlg == PeakPickingAlg::THRESHOLD) {
		_thresholder.addValue(onsetValue, _m->_currentSample);
	}
	if (peakAlg == PeakPickingAlg::CONVOLVE_THEN_THRESHOLD) {
		_thresholder.addValue(_CONVonsetDetectionHistory.newest(), _m->_currentSample);
	}

	Peak lastPeak;
	if (_thresholder.getLastPeak(10, lastPeak)) {
		_onsetPeaks.add(lastPeak);

		_displayPeaks.add(0.999f);
	}
	else {
		_displayPeaks.add(std::max(0.0f, 2 * _displayPeaks.newest() - 1.0f));
	}
}

//*** onset algorithms ***

float NoteOnset::derivativeOfLogEnergy() {

	_energy->calculateNext(_m->_fftHistory.newest(), _m->_fftHistory.numHarmonics()); //depends on energy

	float one_over_dt = ((float)_m->_sampleRate / (float)(_m->_currentSample - _m->_previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//--change of energy per second, using log scale as human ear is log scale
	//good for detecting beats of music with 4 to floor drum pattern

	float derOfLogEnergy = (logf(_energy->getEnergy()) - logf(_energy->getHistory()->previous())); // d(log(E))
	derOfLogEnergy *= one_over_dt; // *= 1/dt
	if (isnan(derOfLogEnergy) || isinf(derOfLogEnergy)) { derOfLogEnergy = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf

	return (derOfLogEnergy < 0 ? 0 : derOfLogEnergy); //only take onset (positive change in energy)

	//--
}


float NoteOnset::bandedDerOfLogEnergy()
{
	float one_over_dt = ((float)_m->_sampleRate / (float)(_m->_currentSample - _m->_previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//same as der of log energy but weight the bands

	//calculate bands -- 
	_derOfLogEnergyBands.getMasterTransform()->beginCalculation();
	_derOfLogEnergyBands.getMasterTransform()->applyFunction(FourierTransform::SMOOTH);
	_derOfLogEnergyBands.getMasterTransform()->applyFunction(FourierTransform::FREQUENCY_CONVOLVE);
	_derOfLogEnergyBands.getMasterTransform()->endCalculation();

	_derOfLogEnergyBands.updateAll(true);
	//--

	float weights[9] = { 1,1,2,2,5,5,10,20,50 };

	float sum = 0;
	for (int i = 0; i < _derOfLogEnergyBands.numBands(); i++) {
		float bandDOLE = (logf(_derOfLogEnergyBands.getBand(i)->getBandEnergy()) - logf(_derOfLogEnergyBands.getBand(i)->getPrevBandEnergy()));
		bandDOLE *= one_over_dt; // *= 1/dt
		if (isnan(bandDOLE) || isinf(bandDOLE)) { bandDOLE = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf
		sum += weights[i] * bandDOLE * _derOfLogEnergyBands.getBand(i)->getBandEnergy();
	}

	return (sum < 0 ? 0 : sum); //only take onset (positive change in energy)
}


float NoteOnset::spectralDistanceOfHarmonics() {
	float one_over_dt = ((float)_m->_sampleRate / (float)(_m->_currentSample - _m->_previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//calculate the fourier transform to use for spectral distance--
	_ftForSpectralDistance.beginCalculation();
	_ftForSpectralDistance.applyFunction(FourierTransform::SMOOTH);
	_ftForSpectralDistance.applyFunction(FourierTransform::FREQUENCY_CONVOLVE);
	_ftForSpectralDistance.endCalculation();
	//--

	//l2 norm of fourier transform--
	float spectralDistanceConvolvedHarmonics = Tools::L2distanceMetricIncDimOnly(
		_ftForSpectralDistance.getHistory()->newest(),
		_ftForSpectralDistance.getHistory()->previous(),
		_ftForSpectralDistance.getHistory()->numHarmonics()
	);
	spectralDistanceConvolvedHarmonics *= one_over_dt;

	return spectralDistanceConvolvedHarmonics;
	//--
}

float NoteOnset::similarityMatrixMelSpectrogram()
{
	_simMatrix.calculateNext(PRECUSSION);
	return _simMatrix.getSimilarityMeasure();
}

//***

void NoteOnset::initSetters()
{
	VisualiserShaderManager::addHistoryAsPossibleSSBOsetter("Note Onset", &_onsetDetectionHistory);
}

void NoteOnset::deleteSetters()
{
	VisualiserShaderManager::deleteHistoryAsPossibleSSBOsetter("Note Onset");
}
