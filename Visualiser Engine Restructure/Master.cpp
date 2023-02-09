#include "Master.h"
#include <Vengine/MyErrors.h>
#include <cmath>

#include "MyFuncs.h"

Master::Master() :
	_audioData(nullptr)
{
}

void Master::init(float* audioData, int sampleRate)
{

	_audioData = audioData;
	_sampleRate = sampleRate;
}

void Master::beginCalculations(int currentSample) {

	_currentSample = currentSample;
}

void Master::endCalculations() {

	_previousSample = _currentSample;
}

void Master::reset() {

	_previousSample = -1;
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