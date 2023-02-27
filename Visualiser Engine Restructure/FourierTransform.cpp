#include "FourierTransform.h"



void FourierTransform::applyFunction(FunctionType type)
{
	if (type == TIME_CONVOLVE) {
		applyFreqConvolving(_current->workingArray(), _next->workingArray());
	}
	if (type == SMOOTH) {
		applySmoothing(_current->workingArray(), _next->workingArray());
	}
	if (type == FREQUENCY_CONVOLVE) {
		applyFreqConvolving(_current->workingArray(), _next->workingArray());
	}


	//swap current and next
	auto tmp = _current;
	_current = _next;
	_next = tmp;
}

void FourierTransform::applyFunctions(FunctionType* args, int numArgs)
{
	for (int i = 0; i < numArgs; i++) {
		applyFunction(args[i]);
	}
}


void FourierTransform::applyFreqConvolving(float* in, float* out)
{
	if (_windowSize == 1) {
		return;
	}

	memset(out, 0.0f, sizeof(float) * _numHarmonics);

	for (int i = 0; i < _numHarmonics; i++) {
		int jStart = std::max(i - (_windowSize / 2), 0);
		for (int j = jStart; j < std::min(i + (_windowSize / 2), _numHarmonics); j++) {
			out[i] += in[j] * Kernels::apply(_freqKernel, j - jStart, _windowSize);
		}
	}
}

void FourierTransform::applySmoothing(float* in, float* out) {
	if (_m->_previousSample == -1) {
		return; //need 2 samples
	}

	//calculate smoothed ft from 'in' ft
	float timeSinceLastCalculation = float(_m->_currentSample - _m->_previousSample) / float(_m->_sampleRate);
	float maxAcceleration = _maxAccelerationPerSecond * timeSinceLastCalculation;

	for (int i = 0; i < _numHarmonics; i++) {
		if (in[i] > _smoothedFt[i]) {
			float change = std::fmin(timeSinceLastCalculation / _attack, in[i] - _smoothedFt[i]);
			change = std::fmin(_smoothedFtDot[i] + maxAcceleration, change); //clamp change to be within max acceleration of last change
			_smoothedFtDot[i] = change;
		}
		else {
			float change = -std::fmin(timeSinceLastCalculation / _release, _smoothedFt[i] - in[i]);
			change = std::fmax(_smoothedFtDot[i] - maxAcceleration, change);
			_smoothedFtDot[i] = change;
		}
		_smoothedFt[i] += _smoothedFtDot[i];
		if (_smoothedFt[i] < 0) {
			_smoothedFt[i] = 0;
		}
	}

	//set out
	memcpy(out, _smoothedFt, sizeof(float) * _numHarmonics);
}

void FourierTransform::applyTimeConvolving(FourierTransformHistory* in, float* out) {

	//sanity check input
	if (_previousXtransforms > _historySize) {
		Vengine::warning("Requested to convolve over more frames than exists stored fft data, defaulting to over all stored data");
		_previousXtransforms = _historySize;
	}

	memset(out, 0.0f, sizeof(float) * _numHarmonics);

	for (int i = 0; i < _previousXtransforms; i++) {
		for (int j = 0; j < _numHarmonics; j++) {
			out[j] += in->get(i)[j] * Kernels::apply(_timeKernel, i, _previousXtransforms); //overwrite oldest
		}
	}
}