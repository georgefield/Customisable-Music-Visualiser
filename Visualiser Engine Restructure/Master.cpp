#include "Master.h"
#include <Vengine/MyErrors.h>
#include <cmath>
#include <functional>

#include "VisualiserShaderManager.h"

Master::Master() :
	_audioData(nullptr),

	_fftwAPI(4096), 
	_fftHistory(17),//store 17 previous fourier transforms

	_sampleFftLastCalculated(-1)
{
}

void Master::init(float* audioData, int sampleRate)
{
	_fftHistory.init(_fftwAPI.numHarmonics());

	_audioData = audioData;
	_sampleRate = sampleRate;
}

void Master::beginCalculations(int currentSample) {

	if (currentSample == _previousSample) {
		Vengine::warning("No change in sample between begin calculation calls");
	}
	_currentSample = currentSample;
}


float slidingWindowFunction(float frac){ //reduces noise
	return -0.5f * cosf(2.0f * 3.1415926f * frac) + 0.5f; //hanning function, increases min detectable frequency by 2, [worst case 24hz]
}

void Master::calculateFourierTransform() {
	//make sure not called twice on same frame
	if (_sampleFftLastCalculated == _currentSample) {
		return;
	}
	_sampleFftLastCalculated = _currentSample;

	_fftwAPI.calculate(_audioData, _currentSample, _fftHistory.workingArray(), 8.0f, slidingWindowFunction); //use fftw api to calculate fft
	_fftHistory.addWorkingArrayToHistory();
	//updates _fftHistory ^^^
}


void Master::endCalculations() {

	_previousSample = _currentSample;
}

void Master::reset() {

	_previousSample = -1;
	_sampleFftLastCalculated = -1;
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
	return conv;
}