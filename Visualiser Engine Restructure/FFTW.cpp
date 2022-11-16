#include "FFTW.h"
#include <Vengine/Errors.h>

FFTW::FFTW(int N) :
	_p(nullptr),
	_out(nullptr)
{
	_N = N;
	_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * ((_N / 2) + 1));
}

void FFTW::getFFT(float* samples, int currentSample, std::vector<float>& harmonicValues, float divFac)
{
	if (harmonicValues.size() < ((_N / 2) + 1)) {
		Vengine::fatalError("Out vector not big enough for fourier transform");
	}

	_p = fftwf_plan_dft_r2c_1d(_N, &(samples[currentSample]), _out, FFTW_ESTIMATE);

	fftwf_execute(_p);
	fftwf_destroy_plan(_p);

	for (int i = 0; i < ((_N / 2) + 1); i++) {
		harmonicValues[i] = sqrtf((_out[i][0] * _out[i][0]) + (_out[i][1] * _out[i][1])) / divFac;
	}
}

FFTW::~FFTW() {
	fftwf_free(_out);
}