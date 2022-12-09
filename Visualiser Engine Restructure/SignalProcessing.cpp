#include "SignalProcessing.h"
#include <Vengine/MyErrors.h>

#include "MyFuncs.h"

const int N = 4096; //sample window for calculations

SignalProcessing::SignalProcessing(int flags) :
	_audioData(nullptr),
	_fft(N),
	_previousSample(-1),
	_state(flags),
	_sumOfSamplesSquared(0)
{
	_fourierHarmonics.resize((N / 2) + 1);
}

void SignalProcessing::update(int currentSample) {

	if (_audioData == nullptr) { Vengine::warning("Attempted to update signal processing class with no attached audio data"); return; }
	if (_state & DISABLE_ALL) { return; } //nothing to do

	if (_state & ENABLE_FOURIER_CALCULATION) {
		_fft.getFFT(_audioData, currentSample, _fourierHarmonics, 500);
	}

	if (_state & ENABLE_RMS_CALCULATION) {
		RMS(currentSample);
		MyFuncs::updateRMS(_rms);
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