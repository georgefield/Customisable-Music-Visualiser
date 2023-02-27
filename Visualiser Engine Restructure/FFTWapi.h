#pragma once
#include <fftw3.h>

class FFTWfft
{
public:
	FFTWfft(int windowSize);
	void calculate(float* audioData, int currentSample, float* store, float gain = 1.0f, float (*slidingWindowFunction)(float) = nullptr);
	~FFTWfft();
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

class FFTWdct {
public:
	FFTWdct();
	void init(int windowSize);
	void calculate(float* dataIn);
	//getters
	float* getOutput() const { return _out; }
	int windowSize() const { return _windowSize; }
private:
	float* _out;
	fftwf_plan _p;
	int _windowSize;
};
