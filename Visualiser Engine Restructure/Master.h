#pragma once
#include <vector>
#include <map>
#include <GL/glew.h>
#include <Vengine/Audio.h>

#include "FFTWapi.h"
#include "History.h"
#include "FourierTransformHistory.h"
#include "Kernels.h"

#include <cassert>

/// <summary>
/// class that contains all basic calculation information used in many signal processing algorithms
/// also contains helper functions that are used a lot with audio maths
/// also does the base fft transform
/// </summary>

class Master {
public:
	Master();
	~Master();

	void init(float* audioData, int sampleRate, bool initSetters = true);
	void reInit(float* audioData, int sampleRate);

	void beginCalculations(int currentSample);
	void endCalculations();

	float* _audioData;
	int _sampleRate;
	int _previousSample;
	int _currentSample;
	float _peakAmplitude;
	float _peakAmplitudeDb;
	float _energy;
	float _RMS;
	FourierTransformHistory _fftHistory;

	void calculateFourierTransform();
	void calculateEnergy();
	void calculatePeakAmplitude();
	void audioIsPaused();

	//helper functions
	float sumOfConvolutionOfHistory(History<float>* history, int entries = 0, Kernel kernel = LINEAR_PYRAMID);

	float* getBaseFftOutput();
	int getBaseFftNumHarmonics();
	float getEnergy();
	float getPeakAmplitude();
	float getPeakAmplitudeDb();
	float getRMS();

	int nyquist() const { assert(_sampleRate > 0); return _sampleRate / 2; }
private:
	int _sampleFftLastCalculated;
	FFTWfft _fftwAPI;

	int _sampleOfLastPeak;

	bool _useSetters;

	void setUpdaters();
	void removeUpdaters();
};