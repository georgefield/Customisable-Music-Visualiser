#pragma once
#include <vector>
#include <map>
#include "FFTW.h"
#include <GL/glew.h>
#include "History.h"
#include "Kernels.h"



enum SPflags {
	NONE = 0,
	FOURIER_CALCULATION = 1,
	RMS_CALCULATION = 2,
	ONSET_DETECTION = 4,
	TEMPO_DETECTION = 8,


	ALL = 255 //2^x - 1, =1111 1111, will trigger all
};

class SignalProcessing {
public:
	SignalProcessing();
	void init(float* audioData, int sampleRate, int flags = ALL);

	void enable(int flags) { _state |= flags; }
	void disable(int flags) { _state &= ~flags; } //& with inverted bits of flag

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
	History<float> _CONVderOfLogEnergy;
	History<float> _CONVspectralDistance;
	History<float*> _convolvedFourierHarmonics;
	History<float> _spectralDistanceConvolvedHarmonics;
	History<float> _CONVspectralDistanceConvolvedHarmonics;

	History<float> _BRUH;

	std::vector<int> _peaks;


	//*** signal processing functions ***

	void RMS(int currentSample);
	void energy(int currentSample, Kernel kernel);

	//note onset
	void noteOnset(int currentSample);
	void peakPicking(int currentSample, History<float>* data);

	void tempo(int currentSample);

	void convolveFourierHarmonics(float* out, Kernel kernel = LINEAR_PYRAMID);

	//***
};