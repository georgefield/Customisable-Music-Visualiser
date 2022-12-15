#include "SignalProcessing.h"
#include <Vengine/MyErrors.h>
#include <cmath>

#include "MyFuncs.h"

const int N = 4096; //sample window for calculations
const int Hs = 2048;

SignalProcessing::SignalProcessing() :
	_state(NONE),
	_audioData(nullptr),
	_fft(N),
	_previousSample(-1),
	_sumOfSamplesSquared(0),
	_spectralDistance(Hs),
	_energy(Hs),
	_derOfLogEnergy(Hs),
	_fftOutput(_fft.getHistory()),
	_convolvedFourierHarmonics(17),
	_CONVderOfLogEnergy(Hs),
	_CONVspectralDistance(Hs),
	_spectralDistanceConvolvedHarmonics(Hs),
	_CONVspectralDistanceConvolvedHarmonics(Hs)
{
}

void SignalProcessing::init(float* audioData, int sampleRate, int flags)
{
	_audioData = audioData;
	_sampleRate = sampleRate;
	_state = flags;

	for (int i = 0; i < _convolvedFourierHarmonics.totalSize(); i++) { //allocate memory for history of fft with convolved harmonics
		float* ptr = (float*)malloc(_fft.numHarmonics() * sizeof(float));
		memset(ptr, 0.0f, _fft.numHarmonics() * sizeof(float));
		_convolvedFourierHarmonics.add(ptr);
	}
}

void SignalProcessing::update(int currentSample) {

	if (_audioData == nullptr) { Vengine::warning("Attempted to update signal processing class with no attached audio data"); return; }
	if (_state == NONE) { return; } //_state = 0

	if (_state & FOURIER_CALCULATION) {
		_fft.getFFT(_audioData, currentSample, 500);
	}

	if (_state & RMS_CALCULATION) {
		RMS(currentSample);
		MyFuncs::updateRMS(_rms);
	}

	if (_state & ONSET_DETECTION) {
		energy(currentSample, LINEAR_PYRAMID);
		noteOnset(currentSample);
	}

	_previousSample = currentSample;
}

void SignalProcessing::reset() {
	_previousSample = -1;
	_rms = 0;
	_sumOfSamplesSquared = 0;

	//reset histories apart from fft output history
	_energy.clear();
	_spectralDistance.clear();
	_derOfLogEnergy.clear();
	_spectralDistanceConvolvedHarmonics.clear();
	_CONVderOfLogEnergy.clear();
	_CONVspectralDistance.clear();

	for (int i = 0; i < _convolvedFourierHarmonics.totalSize(); i++) {
		memset(_convolvedFourierHarmonics.get(i), 0.0f, _convolvedFourierHarmonics.totalSize() * sizeof(float));
	}
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

//used to create a new history which is smoothed
//better than a rolling average as big individual values are first added with a small constant (for hill shaped kernels atleast)
float convolveHistoryWithKernel(History<float>* history, int entries = 0, Kernel kernel = LINEAR_PYRAMID) {

	if (entries == 0) entries = history->totalSize();//default convolve all history

	float conv = 0;
	for (int i = 0; i < entries; i++) {
		conv += history->get(i) * Kernels::apply(kernel, i, entries); //integral of the multiplication = dot product   (in discrete space)
	}
	return conv * 2.0f / entries;
}

void SignalProcessing::energy(int currentSample, Kernel kernel) {

	float sum = 0;
	for (int i = currentSample; i < currentSample + N; i++) {
		sum += fabsf(_audioData[i]) * Kernels::apply(kernel, i - currentSample, N);
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
float L2normIncOnly(float* v1, float* v2, int dim) {

	float ret = 0;
	for (int i = 0; i < dim; i++) {
		if (v1[i] - v2[i] > 0) {
			ret += (v1[i] - v2[i]) * (v1[i] - v2[i]);
		}
	}
	return ret;
}


void SignalProcessing::convolveFourierHarmonics(float* out, Kernel kernel) {

	memset(out, 0.0f, _fft.numHarmonics() * sizeof(float));

	float normalisingGuess = (2.0f / _fftOutput->totalSize());

	for (int i = 0; i < _fftOutput->totalSize(); i++) {
		for (int j = 0; j < _fft.numHarmonics(); j++) {
			out[j] += _fftOutput->get(i)[j] * Kernels::apply(kernel, i, _fftOutput->totalSize()) * normalisingGuess;
		}
	}
}

void SignalProcessing::noteOnset(int currentSample) {

	float one_over_dt = ((float)_sampleRate / (float)(currentSample - _previousSample)) * 0.001; //do dt in ms as otherwise numbers too big

	//	***temporal features***

	//--change of energy per second, using log scale as human ear is log scale
	//good for detecting beats of music with 4 to floor drum pattern

	float derOfLogEnergy = (logf(_energy.get(0)) - logf(_energy.get(1))); // d(log(E))
	derOfLogEnergy *= one_over_dt; // *= 1/dt
	if (isnan(derOfLogEnergy) || isinf(derOfLogEnergy)) { derOfLogEnergy = 0; } //if an energy is 0, log is -inf, stops cascade of nan/inf
	_derOfLogEnergy.add((derOfLogEnergy < 0 ? 0 : derOfLogEnergy)); //only take onset (positive change in energy)

	_CONVderOfLogEnergy.add(convolveHistoryWithKernel(&_derOfLogEnergy, 20, LINEAR_PYRAMID));
	//--

	//	***spectral features***
	
	//--spectral distance, take fourier transform as N dimension point, calculates L2norm from previous transform to current
	float spectralDistance = L2normIncOnly(_fftOutput->get(0), _fftOutput->get(1), _fft.numHarmonics());
	spectralDistance *= one_over_dt; //must be scaled for time
	_spectralDistance.add(spectralDistance);

	_CONVspectralDistance.add(convolveHistoryWithKernel(&_spectralDistance, 20, LINEAR_PYRAMID));
	//--

	//--spectral distance of convolved harmonics, may be slow
	convolveFourierHarmonics(_convolvedFourierHarmonics.get(0), LINEAR_PYRAMID);
	_convolvedFourierHarmonics.add(_convolvedFourierHarmonics.oldest()); //increment history without changing pointers

	float spectralDistanceConvolvedHarmonics = L2normIncOnly(_convolvedFourierHarmonics.get(0), _convolvedFourierHarmonics.get(1), _fft.numHarmonics());
	spectralDistanceConvolvedHarmonics *= one_over_dt;
	_spectralDistanceConvolvedHarmonics.add(spectralDistanceConvolvedHarmonics);

	_CONVspectralDistanceConvolvedHarmonics.add(convolveHistoryWithKernel(&_spectralDistanceConvolvedHarmonics, 10, LINEAR_PYRAMID));
	//--
	//^^^problem with spectral distance is that offset are hard to differentiate from onset as distance depends on how different the spectrum is
	//   somewhat solved by only including positive difference in harmonics (increasing volume), although this only really works for convolved harmonics as
	//   otherwise too much noise (this means spectral distance with non-convolved harmonics will probably not be used)


	//	***neg log likelihood***

	//[TBC]


}

void SignalProcessing::peakPicking(int currentSample, float* data)
{

}

void SignalProcessing::updateSSBOwithHistory(History<float>* history, GLuint id, GLint binding) {

	Vengine::DrawFunctions::updateSSBOpart(id, binding, history->firstPartPtr(), 0, history->firstPartSize() * sizeof(float));
	Vengine::DrawFunctions::updateSSBOpart(id, binding, history->secondPartPtr(), history->firstPartSize() * sizeof(float), history->secondPartSize() * sizeof(float));
}

