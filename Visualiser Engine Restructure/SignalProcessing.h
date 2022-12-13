#pragma once
#include <vector>
#include "FFTW.h"
#include <GL/glew.h>
#include "History.h"

enum SPflags {
	NONE = 0,
	ALL = 1,
	FOURIER_CALCULATION = 2,
	RMS_CALCULATION = 4,
	ONSET_DETECTION = 8
};

class SignalProcessing {
public:
	SignalProcessing(int sampleRate, int flags = ALL);

	void enable(int flags) { _state |= flags; }
	void disable(int flags) { _state &= ~flags; } //& with inverted bits of flag
	void setAudioData(float* audioData) { _audioData = audioData; }

	void update(int currentSample);
	void reset(); //there has been a break in calculations

	void updateSSBOwithHistory(History<float>* history, GLuint id, GLint binding);

	//getters
	float* getFourierHarmonics() { return _fftOutput->newest(); }
	int getHowManyHarmonics() { return _fft.numHarmonics(); }
	float getRMS() { return _rms; } 
	History<float>* getEnergyHistory() { return &_energy; }
//private:
	int _state;
	float* _audioData;
	int _sampleRate;

	FFTW _fft;
	int _previousSample;

	//signal processing outputs
	History<float*>* _fftOutput;
	float _sumOfSamplesSquared;
	float _rms;
	History<float> _energy;
	History<float> _spectralDistance;
	History<float> _derOfLogEnergy;

	//signal processing functions
	void RMS(int currentSample);
	void energy(int currentSample);
	void noteOnset(int currentSample);
};