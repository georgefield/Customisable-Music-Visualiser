#pragma once
#include <vector>
#include "FFTW.h"

enum SPflags {
	DISABLE_ALL,
	ENABLE_FOURIER_CALCULATION,
	DISABLE_FOURIER_CALCULATION,
	ENABLE_RMS_CALCULATION,
	DISABLE_RMS_CALCULATION
};

class SignalProcessing {
public:
	SignalProcessing(int flags = DISABLE_ALL);

	void enable(int flags) { _state |= flags; }
	void disable(int flags) { _state &= ~flags; } //& with inverted bits of flag
	void setAudioData(float* audioData) { _audioData = audioData; }

	void update(int currentSample);
	void reset(); //there has been a break in calculations

	//getters
	float* getFourierHarmonics() { return &(_fourierHarmonics[0]); }
	int getHowManyHarmonics() { return _fourierHarmonics.size(); }
	float getRMS() { return _rms; }
private:
	int _state;
	float* _audioData;

	FFTW _fft;
	int _previousSample;

	//signal processing outputs
	std::vector<float> _fourierHarmonics;
	float _sumOfSamplesSquared;
	float _rms;

	//signal processing functions
	void RMS(int currentSample);
};