#include "FFTW.h"
#include <Vengine/MyErrors.h>

FFTW::FFTW(int windowSize) : //N is window size
	_p(nullptr),
	_out(nullptr),
	_windowSize(windowSize),
	_numHarmonics((windowSize / 2) + 1)
{
	_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _numHarmonics);
	_samplesAfterWindowFunction = new float[_windowSize];
}

void FFTW::calculate(float* audioData, int currentSample, float* storage, float gain, float (*slidingWindowFunction)(float))
{
	//sliding window function = nullptr => no window function, use samples no function (rectangle window)
	float* arrayToUse;
	if (slidingWindowFunction == nullptr) {
		arrayToUse = &(audioData[currentSample]); //saves copying samples to another array
	}
	else {
		//if not copy to new array applying window function
		memset(_samplesAfterWindowFunction, 0.0f, sizeof(float) * _windowSize);
		for (int i = 0; i < _windowSize; i++) {
			_samplesAfterWindowFunction[i] = slidingWindowFunction(float(i) / float(_windowSize)) * audioData[currentSample + i];
		}
		arrayToUse = _samplesAfterWindowFunction;
	}

	_p = fftwf_plan_dft_r2c_1d(_windowSize, arrayToUse, _out, FFTW_ESTIMATE);

	fftwf_execute(_p);
	fftwf_destroy_plan(_p);

	for (int i = 0; i < _numHarmonics; i++) {
		storage[i] = sqrtf((_out[i][0] * _out[i][0]) + (_out[i][1] * _out[i][1])) * gain * (float(1)/float(_windowSize)); //1/window size max possible value so normalise by that factor
	}
}

FFTW::~FFTW() {
	fftwf_free(_out);
}