#pragma once
#include <vector>
#include <map>
#include <GL/glew.h>
#include <Vengine/Audio.h>

#include "FFTWapi.h"
#include "History.h"
#include "FourierTransformHistory.h"
#include "Kernels.h"

/// <summary>
/// class that contains all basic calculation information used in many signal processing algorithms
/// also contains helper functions that are used a lot with audio maths
/// also does the base fft transform
/// </summary>

class Master {
public:
	Master();
	~Master();

	void init(float* audioData, int sampleRate);
	void reInit(float* audioData, int sampleRate);

	void beginCalculations(int currentSample);
	void endCalculations();

	float* _audioData;
	int _sampleRate;
	int _previousSample;
	int _currentSample;
	FourierTransformHistory _fftHistory;

	void calculateFourierTransform();

	//helper functions
	float sumOfConvolutionOfHistory(History<float>* history, int entries = 0, Kernel kernel = LINEAR_PYRAMID);

	float* getBaseFftOutput();
	int getBaseFftNumHarmonics();

	int nyquist() const { if (_sampleRate == 0) { Vengine::fatalError("Used master before initialising"); } return _sampleRate / 2; }
private:
	int _sampleFftLastCalculated;
	FFTWfft _fftwAPI;

	void initSetters();
	void deleteSetters();
};