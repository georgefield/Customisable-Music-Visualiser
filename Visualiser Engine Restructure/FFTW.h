#pragma once
#include <vector>
#include <fftw3.h>
#include "History.h"

class FFTW
{
public:
	FFTW(int historySize, int N);
	void calculate(float* samples, int currentSample, float divFac);
	~FFTW();
	float* getFft() { return _fftHistory.newest(); }
	History<float*>* getFftHistory() { return &_fftHistory; }
	int numHarmonics() { return (_N / 2) + 1; }
private:
	History<float*> _fftHistory;
	int _N;
	fftwf_complex* _out;
	fftwf_plan _p;
};

