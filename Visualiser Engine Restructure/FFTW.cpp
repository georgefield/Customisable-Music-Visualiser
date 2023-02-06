#include "FFTW.h"
#include <Vengine/MyErrors.h>

FFTW::FFTW(int N) :
	_p(nullptr),
	_out(nullptr),
	_harmonicValues(17) //store 17 previous fourier transforms, prime number to stop resonance, used for convolving
{
	_N = N;
	_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * ((_N / 2) + 1));


	for (int i = 0; i < _harmonicValues.totalSize(); i++){ //allocate memory for history (init memory to 0)
		float* ptr = (float*)malloc(((_N / 2) + 1) * sizeof(float));
		memset(ptr, 0.0f, ((_N / 2) + 1) * sizeof(float));
		_harmonicValues.add(ptr);
	}
}

float* FFTW::getFFT(float* samples, int currentSample, float divFac)
{

	_p = fftwf_plan_dft_r2c_1d(_N, &(samples[currentSample]), _out, FFTW_ESTIMATE);

	fftwf_execute(_p);
	fftwf_destroy_plan(_p);

	for (int i = 0; i < ((_N / 2) + 1); i++) {
		_harmonicValues.newest()[i] = sqrtf((_out[i][0] * _out[i][0]) + (_out[i][1] * _out[i][1])) / divFac; //set entry
		_harmonicValues.add(_harmonicValues.oldest()); //increment whilst keeping same pointers
	}
	
	return _harmonicValues.newest(); //return just calculated fft
}

FFTW::~FFTW() {
	fftwf_free(_out);
	for (int i = 0; i < _harmonicValues.totalSize(); i++) { //delete memory allocated for history
		delete[] _harmonicValues.get(i);
	}
}