#pragma once
#include <fftw3.h>

class FFTW
{
public:
	FFTW(int N);
	void calculate(float* audioData, int currentSample, float* store, float gain = 1.0f, float (*slidingWindowFunction)(float) = nullptr);
	~FFTW();
	//getters
	int windowSize() const { return _windowSize; }
	int numHarmonics() const { return _numHarmonics; }
private:
	float* _samplesAfterWindowFunction;

	int _windowSize;
	int _numHarmonics;
	fftwf_complex* _out;
	fftwf_plan _p;
};

