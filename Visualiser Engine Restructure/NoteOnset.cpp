#include "NoteOnset.h"


void NoteOnset::calculateNext(DataExtractionAlg dataAlg, bool convolve) {

	//make sure not called twice on same frame
	if (_sampleLastCalculated == _m->_currentSample) {
		return;
	}
	_sampleLastCalculated = _m->_currentSample;

	//onset detection--
	float onsetValue = 0;
	assert(dataAlg <= COMBINATION && dataAlg >= ENERGY);
	if (dataAlg == DataExtractionAlg::ENERGY) {
		onsetValue = energy();
	}
	if (dataAlg == DataExtractionAlg::DER_OF_LOG_ENERGY) {
		onsetValue = derivativeOfLogEnergy();
	}
	if (dataAlg == DataExtractionAlg::BANDED_DER_OF_LOG_ENERGY) {
		onsetValue = bandedDerOfLogEnergy();
	}
	if (dataAlg == DataExtractionAlg::SPECTRAL_DISTANCE || dataAlg == DataExtractionAlg::SPECTRAL_DISTANCE_HFC_WEIGHTED) {
		onsetValue = spectralDistanceOfHarmonics(SPvars._onsetDetectionFunctionEnum == SPECTRAL_DISTANCE_HFC_WEIGHTED);
	}
	if (dataAlg == DataExtractionAlg::SIM_MATRIX_MEL_SPEC) {
		onsetValue = similarityMatrixMelSpectrogram();
	}
	if (dataAlg == DataExtractionAlg::COMBINATION_FAST) {
		onsetValue = combinationFast();
	}
	if (dataAlg == DataExtractionAlg::COMBINATION) {
		onsetValue = combination();
	}

	_onsetDetectionHistory.add(onsetValue, _m->_currentSample);

	if (convolve) {
		_CONVonsetDetectionHistory.add(
			_m->sumOfConvolutionOfHistory(&_onsetDetectionHistory, SPvars._convolveWindowSize, LINEAR_PYRAMID),
			_m->_currentSample
		);
	}
	//--

	//peak detection--
	if (convolve)
		_thresholder.addValue(_CONVonsetDetectionHistory.newest(), _m->_currentSample);
	else
		_thresholder.addValue(onsetValue, _m->_currentSample);

	Peak lastPeak;
	bool aboveThreshold;
	if (_thresholder.getLastPeak(SPvars._thresholdPercentForPeak, lastPeak)) {
		_onsetPeaks.add(lastPeak);
		std::cout << lastPeak.salience << std::endl;
	}

	_displayPeaks.add(_thresholder.currentlyInPeak());
	//--
}

//*** onset algorithms ***
void knee(float& value, float gain, float thres, float knee) {
	value *= gain;
	if (value > thres) {
		value -= thres;
		value /= knee;
		value += thres;
	}
}

float NoteOnset::energy()
{
	_energy.calculateNext(_m->_fftHistory.newest(), _m->_fftHistory.numHarmonics()); //depends on energy
	float energy = _energy.getEnergy();
	knee(energy, 0.015f, 1.0f, 2.0f);
	return energy;
}

float NoteOnset::derivativeOfLogEnergy() {

	_energy.calculateNext(_m->_fftHistory.newest(), _m->_fftHistory.numHarmonics()); //depends on energy

	float one_over_dt = ((float)_m->_sampleRate / (float)(_m->_currentSample - _m->_previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//--change of energy per second, using log scale as human ear is log scale
	//good for detecting beats of music with 4 to floor drum pattern

	float derOfLogEnergy = (logf(_energy.getEnergy()) - logf(_energy.getHistory()->previous())); // d(log(E))
	derOfLogEnergy *= one_over_dt; // *= 1/dt
	if (isnan(derOfLogEnergy) || isinf(derOfLogEnergy)) { derOfLogEnergy = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf

	knee(derOfLogEnergy, 25.0f, 1.0f, 2.0f);

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

	float weights[9] = { 1,1,2,2,5,5,10,25,40 };

	float sum = 0;
	for (int i = 0; i < _derOfLogEnergyBands.numBands(); i++) {
		float bandDOLE = (logf(_derOfLogEnergyBands.getBand(i)->getBandEnergy()) - logf(_derOfLogEnergyBands.getBand(i)->getPrevBandEnergy()));
		bandDOLE *= one_over_dt; // *= 1/dt
		if (isnan(bandDOLE) || isinf(bandDOLE)) { bandDOLE = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf
		sum += weights[i] * bandDOLE;
	}

	knee(sum, 0.33f, 1.0f, 2.0f);

	return (sum < 0 ? 0 : sum); //only take onset (positive change in energy)
}


float NoteOnset::spectralDistanceOfHarmonics(bool HFC) {
	float one_over_dt = ((float)_m->_sampleRate / (float)(_m->_currentSample - _m->_previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//calculate the fourier transform to use for spectral distance--
	_ftForSpectralDistance.beginCalculation();
	_ftForSpectralDistance.applyFunction(FourierTransform::SMOOTH);
	_ftForSpectralDistance.applyFunction(FourierTransform::FREQUENCY_CONVOLVE);
	_ftForSpectralDistance.endCalculation();
	//--

	//make sure enough entries--
	if (_ftForSpectralDistance.getHistory()->entries() < 2) {
		return 0.0f;
	}
	//--

	//l2 norm of fourier transform--
	//first HFC weighted, second not
	float spectralDistanceConvolvedHarmonics;
	if (HFC) {
		spectralDistanceConvolvedHarmonics = Tools::HFCweightedL2distanceMetricIncDimOnly(
			_ftForSpectralDistance.getHistory()->newest(),
			_ftForSpectralDistance.getHistory()->previous(),
			_ftForSpectralDistance.getHistory()->numHarmonics()
		);
		spectralDistanceConvolvedHarmonics *= one_over_dt;

		knee(spectralDistanceConvolvedHarmonics, 10.0f, 1.0f, 2.0f);
	}
	else {
		spectralDistanceConvolvedHarmonics = Tools::L2distanceMetricIncDimOnly(
			_ftForSpectralDistance.getHistory()->newest(),
			_ftForSpectralDistance.getHistory()->previous(),
			_ftForSpectralDistance.getHistory()->numHarmonics()
		);
		spectralDistanceConvolvedHarmonics *= one_over_dt;

		knee(spectralDistanceConvolvedHarmonics, 100.0f, 1.0f, 2.0f);
	}

	return spectralDistanceConvolvedHarmonics;
	//--
}

float NoteOnset::similarityMatrixMelSpectrogram()
{
	_simMatrix.calculateNext(PRECUSSION, 10.0f);

	float measure = _simMatrix.getSimilarityMeasure();
	if (isnan(measure) || isinf(measure)) { return 0.0f; } //stop nan virus escaping the lab

	knee(measure, 12.0f, 1.0f, 2.0f);
	return std::max(measure, 0.0f);
}

float NoteOnset::combinationFast()
{
	//all normalised to be ~1 at peak
	float dole = derivativeOfLogEnergy();
	float sdh = spectralDistanceOfHarmonics(false);
	float e = energy();
	return (dole + 0.5) * (sdh + 0.2) * (e + 0.3) - 0.03;
}

float NoteOnset::combination()
{
	//all normalised to be ~1 at peak
	float bdole = bandedDerOfLogEnergy();
	float sdh = spectralDistanceOfHarmonics(false);
	float e = energy();
	float smms = similarityMatrixMelSpectrogram();
	return (bdole + 0.7) * (sdh + 0.4) * (smms + 0.4) * (e + 0.2) - 0.0224;
}

//***

void NoteOnset::initSetters()
{

}

void NoteOnset::deleteSetters()
{

}
