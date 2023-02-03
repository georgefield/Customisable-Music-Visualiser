#include "Master.h"
#include <Vengine/MyErrors.h>
#include <cmath>

#include "MyFuncs.h"

Master::Master(int sampleWindowForFft, int historySize) :
	_audioData(nullptr),
	_N(sampleWindowForFft),
	_fft(_N),
	_previousSample(-1),

	_fftOutput(nullptr),
	_timeConvolvedFftOutput(17),

	_sampleFftLastCalculated(-1),
	_sampleTimeConvolvedFftLastCalculated(-1)
{
}

void Master::init(float* audioData, int sampleRate)
{
	_fftOutput = _fft.getHistory();

	_audioData = audioData;
	_sampleRate = sampleRate;

	for (int i = 0; i < _timeConvolvedFftOutput.totalSize(); i++) { //allocate memory for history of fft with convolved harmonics
		float* ptr = (float*)malloc(_fft.numHarmonics() * sizeof(float));
		memset(ptr, 0.0f, _fft.numHarmonics() * sizeof(float));
		_timeConvolvedFftOutput.add(ptr);
		printf("BRUH\n");
	}
}

void Master::beginCalculations(int currentSample) {

	_currentSample = currentSample;
}

void Master::endCalculations() {

	_previousSample = _currentSample;
}

void Master::calculateFft() {
	//make sure not called twice on same frame
	if (_sampleFftLastCalculated == _currentSample) {
		return;
	}
	_sampleFftLastCalculated = _currentSample;

	_fft.getFFT(_audioData, _currentSample, 500);
}

void Master::calculateTimeConvolvedFft() {

	//make sure not called twice on same frame
	if (_sampleTimeConvolvedFftLastCalculated == _currentSample) {
		return;
	}
	_sampleTimeConvolvedFftLastCalculated = _currentSample;

	//dependencies
	calculateFft(); //relies on basic fft

	//--may be slow
	float* out = _timeConvolvedFftOutput.newest();

	memset(out, 0.0f, _fft.numHarmonics() * sizeof(float));

	float normalisingGuess = (2.0f / _fftOutput->totalSize());

	for (int i = 0; i < _fftOutput->totalSize(); i++) {
		for (int j = 0; j < _fft.numHarmonics(); j++) {
			out[j] += _fftOutput->get(i)[j] * Kernels::apply(LINEAR_PYRAMID, i, _fftOutput->totalSize()) * normalisingGuess;
		}
	}

	_timeConvolvedFftOutput.add(_timeConvolvedFftOutput.oldest()); //increment history without changing pointers
}


void Master::reset() {

	_previousSample = -1;
	_sampleFftLastCalculated = -1;
	_sampleTimeConvolvedFftLastCalculated = -1;

	for (int i = 0; i < _timeConvolvedFftOutput.totalSize(); i++) {
		memset(_timeConvolvedFftOutput.get(i), 0.0f, _timeConvolvedFftOutput.totalSize() * sizeof(float));
	}
}


//*** helper functions ***

//used to create a new history which is smoothed
//better than a rolling average as big individual values are first added with a small constant (for hill shaped kernels atleast)
float Master::sumOfConvolutionOfHistory(History<float>* history, int entries, Kernel kernel) {

	if (entries == 0) entries = history->totalSize();//default convolve all history

	float conv = 0;
	for (int i = 0; i < entries; i++) {
		conv += history->get(i) * Kernels::apply(kernel, i, entries); //integral of the multiplication = dot product   (in discrete space)
	}
	return conv * 2.0f / entries;
}