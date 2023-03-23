#include "NoteOnset.h"

void compress(float& value, float gain, float thres, float ratio) {
	value *= gain;
	if (value > thres) {
		value -= thres;
		value /= ratio;
		value += thres;
	}
}

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
	if (dataAlg == DataExtractionAlg::HFC_DER_OF_LOG_ENERGY) {
		onsetValue = HFCderivativeOfLogEnergy();
	}
	if (dataAlg == DataExtractionAlg::SPECTRAL_DISTANCE) {
		onsetValue = spectralDistance();
	}
	if (dataAlg == DataExtractionAlg::SPECTRAL_DISTANCE_WITH_PHASE) {
		onsetValue = weightedPhaseDeviation();
	}
	if (dataAlg == DataExtractionAlg::SIM_MATRIX_MFCC) {
		onsetValue = similarityMatrixMFCC();
	}
	if (dataAlg == DataExtractionAlg::COMBINATION_FAST) {
		onsetValue = combinationFast();
	}
	if (dataAlg == DataExtractionAlg::COMBINATION) {
		onsetValue = combination();
	}
	_rawOnsetDetectionHistory.add(onsetValue); //stores raw algorithm output

	//filtering functions
	if (convolve) {
		onsetValue = _m->sumOfConvolutionOfHistory(&_rawOnsetDetectionHistory, SP::vars._convolveWindowSize, LINEAR_PYRAMID);
	}
	compress(onsetValue, SP::vars._detectionFunctionGain, SP::vars._detectionFunctionCompressionThreshold, SP::vars._detectionFunctionCompressionRatio);
	if (SP::vars._clampBetween0and1) {
		onsetValue = std::max(0.0f, std::min(onsetValue, 1.0f));
	}

	_outOnsetDetectionHistory.add(onsetValue); //add filtered output to main detection history
	//--

	//peak detection--
	_thresholder.addValue(_outOnsetDetectionHistory.newest(), _m->_currentSample);

	Peak lastPeak;
	bool aboveThreshold;
	if (_thresholder.getLastPeak(SP::vars._thresholdPercentForPeak, lastPeak)) {
		_onsetPeaks.add(lastPeak);
	}

	_displayPeaks.add(_thresholder.currentlyInPeak());
	//--
}

//*** onset algorithms ***

float NoteOnset::energy()
{
	float energy = _m->_energy;
	compress(energy, 16.0f, 1.0f, 2.0f);
	return energy;
}

float NoteOnset::derivativeOfLogEnergy() {

	float one_over_dt = SP::vars._desiredCPS * 0.001; //do dt in ms as otherwise numbers too big

	//--change of energy per second, using log scale as human ear is log scale
	//good for detecting beats of music with 4 to floor drum pattern

	float derOfLogEnergy = (logf(_m->_energyHistory.newest()) - logf(_m->_energyHistory.previous())); // d(log(E))
	derOfLogEnergy *= one_over_dt; // *= 1/dt
	if (isnan(derOfLogEnergy) || isinf(derOfLogEnergy)) { derOfLogEnergy = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf

	compress(derOfLogEnergy, 25.0f, 1.0f, 2.5f);

	return (derOfLogEnergy < 0 ? 0 : derOfLogEnergy); //only take onset (positive change in energy)

	//--
}


float NoteOnset::HFCderivativeOfLogEnergy()
{
	float one_over_dt = SP::vars._desiredCPS * 0.001; //do dt in ms as otherwise numbers too big

	float HFCenergy = 0.0f;
	for (int k = 0; k < _m->_fftHistory.numHarmonics(); k++) {
		HFCenergy += k * k * _m->_fftHistory.newest()[k] * _m->_fftHistory.newest()[k]; //weight by k for kth harmonic [masri]
	}
	HFCenergy /= SP::vars._masterFTgain * SP::vars._masterFTgain; //fix the gain applied on transform
	HFCenergy *= 2; // miss out the mirrored half of the fourier transform

	float derOfLogHFCenergy = (logf(HFCenergy) - logf(_previousHFCenergy));
	_previousHFCenergy = HFCenergy;

	derOfLogHFCenergy *= one_over_dt;
	if (isnan(derOfLogHFCenergy) || isinf(derOfLogHFCenergy)) { derOfLogHFCenergy = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf

	compress(derOfLogHFCenergy, 15.0f, 1.0f, 2.5f);

	return (derOfLogHFCenergy < 0 ? 0 : derOfLogHFCenergy); //only take onset (positive change in energy)
}


float NoteOnset::spectralDistance() {
	float one_over_dt = SP::vars._desiredCPS * 0.001; //do dt in ms as otherwise numbers too big

	//calculate the fourier transform to use for spectral distance--
	_ftForSpectralDistance.calculateNext();
	//--

	//make sure enough entries--
	if (_ftForSpectralDistance.getHistory()->entries() < 2) {
		return 0.0f;
	}
	//--

	float spectralDistanceConvolvedHarmonics = Tools::L1distanceMetricIncDimOnly(
		_ftForSpectralDistance.getHistory()->newest(),
		_ftForSpectralDistance.getHistory()->previous(),
		_ftForSpectralDistance.getHistory()->numHarmonics()
	);
	spectralDistanceConvolvedHarmonics *= one_over_dt;

	compress(spectralDistanceConvolvedHarmonics, 8.0f, 1.0f, 2.0f);

	return spectralDistanceConvolvedHarmonics;
	//--
}

float argumentDistance(float arg1, float arg2) { //wrap around pi
	static const float pi = 3.1415926;
	if (arg1 > arg2) {
		return std::min(arg1 - arg2, (pi - arg1) + (-pi - arg2));
	}

	return std::min(arg2 - arg1, (pi - arg2) + (-pi - arg1));
}

float NoteOnset::weightedPhaseDeviation() {
	float one_over_dt = SP::vars._desiredCPS * 0.001; //do dt in ms as otherwise numbers too big

	VectorHistory<MyComplex>* complexOutputs = _m->getBaseFftComplexOutputHistory();
	int numHarmonics = complexOutputs->vectorDim();

	float weightedPhaseDeviationSpread = 0;
	for (int k = 0; k < numHarmonics; k++) {

		float phaseDeviation = argumentDistance(complexOutputs->get(0)[k].arg, complexOutputs->get(1)[k].arg) - argumentDistance(complexOutputs->get(1)[k].arg, complexOutputs->get(2)[k].arg); //second derivative

		weightedPhaseDeviationSpread += fabsf(phaseDeviation) * _m->getBaseFftOutput()[k];
	}
	weightedPhaseDeviationSpread /= numHarmonics;

	weightedPhaseDeviationSpread *= one_over_dt;

	compress(weightedPhaseDeviationSpread, 2000.0f, 1.0f, 2.0f);

	return weightedPhaseDeviationSpread;
}

float NoteOnset::similarityMatrixMFCC()
{
	_simMatrix.calculateNext();

	float measure = _simMatrix.matrix.getSimilarityMeasure();
	if (isnan(measure) || isinf(measure)) { return 0.0f; } //stop nan virus escaping the lab

	compress(measure, 30.0f, 1.0f, 2.0f);
	return std::max(measure, 0.0f);
}

float NoteOnset::combinationFast()
{
	//all normalised to be ~1 at peak
	float dole = derivativeOfLogEnergy();
	float sd = spectralDistance();
	float e = energy();
	return (dole + 0.5) * (sd + 0.2) * (e + 0.3) - 0.03;
}

float NoteOnset::combination()
{
	//all normalised to be ~1 at peak
	float HFCdole = HFCderivativeOfLogEnergy();
	float sd = spectralDistance();
	float sdwp = weightedPhaseDeviation();
	float e = energy();
	float smms = similarityMatrixMFCC();
	return (HFCdole + 0.7) * (sdwp + 0.4) * (smms + 0.4) * (e + 0.2) - 0.0224;
}

//***

void NoteOnset::initSetters()
{

}

void NoteOnset::deleteSetters()
{

}
