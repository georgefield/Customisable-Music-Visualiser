#include "FFTWapi.h"
#include <Vengine/MyErrors.h>
#include <cassert>

//*** FFTWfft ***

FFTWfft::FFTWfft(int windowSize) : //N is window size
	_p(nullptr),
	_out(nullptr),
	_myComplexOutputHistory(7),
	_windowSize(windowSize),
	_numHarmonics((windowSize / 2) + 1),
	_calculated(false),
	_complexHistorySampleLastCalculated(-1)
{
	_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _numHarmonics);
	_samplesAfterWindowFunction = new float[_windowSize];
	_myComplexOutputHistory.init(_numHarmonics);
}

void FFTWfft::calculate(float* audioData, int currentSample, float* storage, float gain, float (*slidingWindowFunction)(float))
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

	_calculated = true;
}

FFTWfft::~FFTWfft() {
	fftwf_free(_out);
}


VectorHistory<MyComplex>* FFTWfft::getComplexCoeffsHistory(int currentSample)
{
	assert(_calculated);

	if (currentSample == _complexHistorySampleLastCalculated) { //once per current sample
		return &_myComplexOutputHistory;
	}

	for (int k = 0; k < _numHarmonics; k++) {
		_myComplexOutputHistory.workingArray()[k].set(_out[k][0], _out[k][1]);
	}
	_myComplexOutputHistory.addWorkingArrayToHistory();
	_complexHistorySampleLastCalculated = currentSample;

	return &_myComplexOutputHistory;
}

//*** FFTWdct ***

FFTWdct::FFTWdct() : 
	_p(nullptr),
	_out(nullptr)
{
}

void FFTWdct::init(int windowSize)
{
	_windowSize = windowSize;

	//dct type 2, n in, n out
	_out = (float*)fftwf_malloc(sizeof(float) * _windowSize);
	memset(_out, 0.0f, sizeof(float) * _windowSize);
}

void FFTWdct::calculate(float* dataIn)
{
	_p = fftwf_plan_r2r_1d(_windowSize, dataIn, _out, fftwf_r2r_kind::FFTW_REDFT10, FFTW_ESTIMATE);

	fftwf_execute(_p);
	fftwf_destroy_plan(_p);
}
