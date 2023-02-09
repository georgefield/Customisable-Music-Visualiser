#include "FFTW.h"
#include <Vengine/MyErrors.h>

FFTW::FFTW(int historySize, int N) :
	_p(nullptr),
	_out(nullptr),
	_fftHistory(historySize) //store 17 previous fourier transforms, prime number to stop resonance, used for convolving
{
	_N = N;
	_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * ((_N / 2) + 1));


	for (int i = 0; i < _fftHistory.totalSize(); i++){ //allocate memory for history (init memory to 0)
		float* ptr = (float*)malloc(((_N / 2) + 1) * sizeof(float));
		memset(ptr, 0.0f, ((_N / 2) + 1) * sizeof(float));
		_fftHistory.add(ptr);
	}
}

void FFTW::calculate(float* samples, int currentSample, float divFac)
{

	_p = fftwf_plan_dft_r2c_1d(_N, &(samples[currentSample]), _out, FFTW_ESTIMATE);

	fftwf_execute(_p);
	fftwf_destroy_plan(_p);

	for (int i = 0; i < ((_N / 2) + 1); i++) {
		_fftHistory.oldest()[i] = sqrtf((_out[i][0] * _out[i][0]) + (_out[i][1] * _out[i][1])) / divFac; //overwrite oldest
	}

	_fftHistory.add(_fftHistory.oldest()); //add oldest, makes it newest
}

FFTW::~FFTW() {
	fftwf_free(_out);
	//free memory storing fft history
	for (int i = 0; i < _fftHistory.totalSize(); i++) { //delete memory allocated for history
		delete[] _fftHistory.get(i);
	}
}