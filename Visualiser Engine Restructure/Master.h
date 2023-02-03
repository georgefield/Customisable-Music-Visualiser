#pragma once
#include <vector>
#include <map>
#include "FFTW.h"
#include <GL/glew.h>
#include "History.h"
#include "Kernels.h"

/// <summary>
/// class that contains all basic calculation information used in many signal processing algorithms
/// </summary>

class Master {
public:
	Master(int sampleWindowForFft, int historySize);

	void init(float* audioData, int sampleRate);

	void beginCalculations(int currentSample);
	void endCalculations();

	void calculateFft();
	void calculateTimeConvolvedFft();

	void reset();

	float* _audioData;
	int _sampleRate;
	int _previousSample;
	int _currentSample;
	int _N; //sample window for fft

	History<float*>* _fftOutput;
	History<float*> _timeConvolvedFftOutput;

	//helper function
	float sumOfConvolutionOfHistory(History<float>* history, int entries = 0, Kernel kernel = LINEAR_PYRAMID);

	int getNumHarmonics() { return _fft.numHarmonics(); }
private:
	int _sampleTimeConvolvedFftLastCalculated;
	int _sampleFftLastCalculated;

	FFTW _fft;
};