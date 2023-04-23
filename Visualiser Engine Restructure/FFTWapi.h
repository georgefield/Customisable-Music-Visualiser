#pragma once
#include <fftw3.h>
#include "VectorHistory.h"

struct MyComplex {
	MyComplex() {
		re = 0;
		im = 0;
		mod = 0;
		arg = 0;
	}
	MyComplex(float Re, float Im) {
		set(Re, Im);
	}
	void set(float Re, float Im) {
		re = Re;
		im = Im;
		mod = sqrtf(re * re + im * im);
		if (re != 0)
			arg = atanf(im / re);
	}
	float re;
	float im;
	float mod;
	float arg;
};

class FFTWfft
{
public:
	FFTWfft(int windowSize);
	void calculate(float* audioData, int startPos, float* store, float gain = 1.0f, float (*slidingWindowFunction)(float) = nullptr);
	~FFTWfft();
	VectorHistory<MyComplex>* getComplexCoeffsHistory(int currentSample);

	//getters
	int windowSize() const { return _windowSize; }
	int numHarmonics() const { return _numHarmonics; }
private:
	float* _samplesAfterWindowFunction;

	int _windowSize;
	int _numHarmonics;
	fftwf_complex* _out;

	VectorHistory<MyComplex> _myComplexOutputHistory;
	int _complexHistorySampleLastCalculated;

	fftwf_plan _p;

	bool _calculated;
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
