#include "SignalProcessing.h"
#include <Vengine/MyErrors.h>

#include "MyFuncs.h"

const int N = 4096; //sample window for calculations
const int Hs = 2048;

SignalProcessing::SignalProcessing(int sampleRate, int flags) :
	_sampleRate(sampleRate),
	_audioData(nullptr),
	_fft(N),
	_previousSample(-1),
	_state(flags),
	_sumOfSamplesSquared(0),
	_spectralDistance(Hs),
	_energy(Hs),
	_derOfLogEnergy(Hs),
	_fftOutput(_fft.getHistory())
{
}

void SignalProcessing::update(int currentSample) {

	if (_audioData == nullptr) { Vengine::warning("Attempted to update signal processing class with no attached audio data"); return; }
	if (_state == NONE) { return; } //_state = 0

	if (_state & (FOURIER_CALCULATION | ALL)) {
		_fft.getFFT(_audioData, currentSample, 500);
	}

	if (_state & (RMS_CALCULATION | ALL)) {
		RMS(currentSample);
		MyFuncs::updateRMS(_rms);
	}

	if (_state & (ONSET_DETECTION | ALL)) {
		energy(currentSample);
		noteOnset(currentSample);
	}

	_previousSample = currentSample;
}

void SignalProcessing::reset() {
	_previousSample = -1;
	_rms = 0;
	_sumOfSamplesSquared = 0;
}


void SignalProcessing::RMS(int currentSample) {

	if (_previousSample != -1 && currentSample - _previousSample < N) { //save compute power if window only adjusted by a bit
		for (int i = _previousSample; i < currentSample; i++) {
			_sumOfSamplesSquared -= _audioData[i] * _audioData[i];
		}

		for (int i = _previousSample + N; i < currentSample + N; i++) {
			_sumOfSamplesSquared += _audioData[i] * _audioData[i];
		}
	}
	else {
		for (int i = currentSample; i < currentSample + N; i++) {
			_sumOfSamplesSquared += _audioData[i] * _audioData[i];
		}
	}

	_rms = sqrtf(_sumOfSamplesSquared / N);
}

float kernel(int m, int n) {
	return 1 - (2.0f/n)*fabsf(m - (n / 2)); //quadratic pyramid kernel (placeholder, should work)
}
void SignalProcessing::energy(int currentSample) {

	float sum = 0;

	for (int i = currentSample; i < currentSample + N; i++) {
		sum += fabsf(_audioData[i]) * kernel(i - currentSample, N);
	}

	_energy.add(sum / N);
}

float L1norm(float* v1, float* v2, int dim) {

	float ret = 0;
	for (int i = 0; i < dim; i++) {
		ret += fabsf(v1[i] - v2[i]);
	}
	return ret;
}
float L2norm(float* v1, float* v2, int dim) {

	float ret = 0;
	for (int i = 0; i < dim; i++) {
		ret += (v1[i] - v2[i]) * (v1[i] - v2[i]);
	}
	return ret;
}
void SignalProcessing::noteOnset(int currentSample) {

	//temporal feature, change of energy per second, using log scale as human ear is log scale
	float derOfLogEnergy = (logf(_energy.get(0)) - logf(_energy.get(1))) * ((currentSample - _previousSample) / _sampleRate); //= / (_sampleRate / (currentSample - _previousSample); = d(log(E))/dt
	_derOfLogEnergy.add((derOfLogEnergy < 0 ? 0 : derOfLogEnergy) * 0.1); //only take onset (positive change in energy)

	//spectral feature (very very slow i think, just testing)
	float sum = 0;
	for (int i = 0; i < _fftOutput->totalSize() - 1; i++) {
		sum += L2norm(_fftOutput->get(i), _fftOutput->get(i + 1), _fft.numHarmonics());
	}
	_spectralDistance.add(sum / (_fftOutput->totalSize() - 1));
}

void SignalProcessing::updateSSBOwithHistory(History<float>* history, GLuint id, GLint binding) {

	Vengine::DrawFunctions::updateSSBOpart(id, binding, history->firstPartPtr(), 0, history->firstPartSize() * sizeof(float));
	Vengine::DrawFunctions::updateSSBOpart(id, binding, history->secondPartPtr(), history->firstPartSize() * sizeof(float), history->secondPartSize() * sizeof(float));
}

