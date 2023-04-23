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

	void init(int sampleRate, bool initSetters = true);
	void reInit(int sampleRate);

	void beginCalculations(int calculationSample, float* audioDataPtr, int audioDataLength);
	void endCalculations();

	float* _audioDataPtr;
	int _audioDataLength;

	int _sampleRate;
	int _previousSample;
	int _currentSample;

	float _peakAmplitude;
	float _peakAmplitudeDb;
	float _energy;
	History<float> _energyHistory;
	float _RMS;
	FourierTransformHistory _fftHistory;

	void calculateFourierTransform();
	void calculateEnergy();
	void calculatePeakAmplitude();
	void audioIsPaused();

	//helper functions
	float sumOfConvolutionOfHistory(History<float>* history, int entries = 0, Kernel kernel = LINEAR_PYRAMID);

	float* getBaseFftOutput();
	MyComplex* getBaseFftComplexOutput();
	VectorHistory<MyComplex>* getBaseFftComplexOutputHistory();

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