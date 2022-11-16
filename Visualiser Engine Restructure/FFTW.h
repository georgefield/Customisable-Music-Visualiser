#pragma once
#include <vector>
#include <fftw3.h>

class FFTW
{
public:
	FFTW(int N);
	void getFFT(float* samples, int currentSample, std::vector<float>& out, float divFac);
	~FFTW();
private:
	int _N;
	fftwf_complex* _out;
	fftwf_plan _p;
};

