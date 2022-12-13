#pragma once
#include <vector>
#include <fftw3.h>
#include "History.h"

class FFTW
{
public:
	FFTW(int N);
	float* getFFT(float* samples, int currentSample, float divFac);
	~FFTW();
	History<float*>* getHistory() { return &_harmonicValues; }
	int numHarmonics() { return (_N / 2) + 1; }
private:
	History<float*> _harmonicValues;
	int _N;
	fftwf_complex* _out;
	fftwf_plan _p;
};

