#include "NoteOnset.h"
#include "MyMaths.h"

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
	if (dataAlg == DataExtractionAlg::COMBINATION) {
		onsetValue = combination();
	}
	_rawOnsetDetectionHistory.add(onsetValue); //stores raw algorithm output

	//filtering functions--
	if (convolve) {

		Kernel kernel;
		kernel.setUp(LINEAR_PYRAMID, Vis::vars._convolveWindowSize);

		onsetValue = 0;
		for (int i = 0; i < Vis::vars._convolveWindowSize; i++) {
			onsetValue += _rawOnsetDetectionHistory.get(i) * kernel.getValueAt(i);
		}
	}

	compress(onsetValue, Vis::vars._detectionFunctionGain, Vis::vars._detectionFunctionCompressionThreshold, Vis::vars._detectionFunctionCompressionRatio);
	
	if (Vis::vars._clampBetween0and1) {
		onsetValue = std::max(0.0f, std::min(onsetValue, 1.0f));
	}

	_outOnsetDetectionHistory.add(onsetValue); //add filtered output to main detection history
	//--

	//peak detection--
	_thresholder.addValue(_outOnsetDetectionHistory.newest(), _m->_currentSample);

	Peak lastPeak;
	bool aboveThreshold;
	if (_thresholder.getLastPeak(Vis::vars._thresholdPercentForPeak, lastPeak)) {
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

	float one_over_dt = Vis::vars._desiredCPS * 0.001; //do dt in ms as otherwise numbers too big

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
	float one_over_dt = Vis::vars._desiredCPS * 0.001; //do dt in ms as otherwise numbers too big

	float HFCenergy = 0.0f;
	for (int k = 0; k < _m->_fftHistory.numHarmonics(); k++) {
		HFCenergy += k * k * _m->_fftHistory.newest()[k] * _m->_fftHistory.newest()[k]; //weight by k for kth harmonic [masri]
	}
	HFCenergy /= Vis::vars._masterFTgain * Vis::vars._masterFTgain; //fix the gain applied on transform
	HFCenergy *= 2; // miss out the mirrored half of the fourier transform

	float derOfLogHFCenergy = (logf(HFCenergy) - logf(_previousHFCenergy));
	_previousHFCenergy = HFCenergy;

	derOfLogHFCenergy *= one_over_dt;
	if (isnan(derOfLogHFCenergy) || isinf(derOfLogHFCenergy)) { derOfLogHFCenergy = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf

	compress(derOfLogHFCenergy, 15.0f, 1.0f, 2.5f);

	return (derOfLogHFCenergy < 0 ? 0 : derOfLogHFCenergy); //only take onset (positive change in energy)
}


float NoteOnset::spectralDistance() {
	float one_over_dt = Vis::vars._desiredCPS * 0.001; //do dt in ms as otherwise numbers too big

	//make sure enough entries--
	if (_m->_fftHistory.entries() < 2) {
		return 0.0f;
	}
	//--

	float spectralDistanceConvolvedHarmonics = MyMaths::L1distanceMetricIncDimOnly(
		_m->_fftHistory.newest(),
		_m->_fftHistory.previous(),
		_m->_fftHistory.numHarmonics()
	);
	spectralDistanceConvolvedHarmonics *= one_over_dt;

	compress(spectralDistanceConvolvedHarmonics, 5.0f, 1.0f, 2.0f);

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
	float one_over_dt = Vis::vars._desiredCPS * 0.001; //do dt in ms as otherwise numbers too big

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


float lerp(float v1, float v2, float t) {
	return (1 - t)*v1 + t * v2;
}

float NoteOnset::combination()
{
	//all normalised to be ~1 at peak
	float e = 0;
	float dole = 0;
	float HFCdole = 0;
	float sd = 0;
	float wpd = 0;

	if (Vis::vars._energyMixAmount > 0)
		e = energy();
	if (Vis::vars._doleMixAmount > 0)
		dole = derivativeOfLogEnergy();
	if (Vis::vars._HFCdoleMixAmount > 0)
		HFCdole = HFCderivativeOfLogEnergy();
	if (Vis::vars._sdMixAmount > 0)
		sd = spectralDistance();
	if (Vis::vars._wpdMixAmount > 0)
		wpd = weightedPhaseDeviation();

	float combination =
		lerp(1, e, Vis::vars._energyMixAmount) *		//energy
		lerp(1, dole, Vis::vars._doleMixAmount) *		//derivative of log energy
		lerp(1, HFCdole, Vis::vars._HFCdoleMixAmount) * //high frequency weightedd erivative of log energy
		lerp(1, sd, Vis::vars._sdMixAmount) *			//spectral distance
		lerp(1, wpd, Vis::vars._wpdMixAmount)			//weighted phase deviation
		- Vis::vars._leftoverConstantFactor;			//- possible constant factor
	return combination;
}

//***

void NoteOnset::initSetters()
{

}

void NoteOnset::deleteSetters()
{

}
