#pragma once
#include <vector>
#include <map>
#include "FFTW.h"
#include <GL/glew.h>
#include "History.h"
#include "FourierTransformHistory.h"
#include "Kernels.h"
#include "FFTW.h"

/// <summary>
/// class that contains all basic calculation information used in many signal processing algorithms
/// also contains helper functions that are used a lot with audio maths
/// </summary>

struct Master {
	Master();

	void init(float* audioData, int sampleRate);

	void beginCalculations(int currentSample);
	void endCalculations();

	void reset();

	float* _audioData;
	int _sampleRate;
	int _previousSample;
	int _currentSample;
	FourierTransformHistory _fftHistory;

	void calculateFourierTransform();

	//helper functions
	float sumOfConvolutionOfHistory(History<float>* history, int entries = 0, Kernel kernel = LINEAR_PYRAMID);

private:
	int _sampleFftLastCalculated;
	FFTW _fftwAPI;

};