#include "FFTs.h"


void FFTs::init(Master* master) {
	_m = master;

	//set up smoothed fft dot (used to limit acceleration)
	_smoothedFftDot = new float[_numHarmonics];
	memset(_smoothedFftDot, 0.0f, _numHarmonics * sizeof(float));

	_fftHistoryPtr = _fftwAPI.getFftHistory(); //set _fftHistoryPtr to point to the history maintained in the fftwAPI

	for (int i = 0; i < _timeConvolvedFftHistory.totalSize(); i++) { //allocate memory for convolve history (init memory to 0)
		float* ptr = (float*)malloc(_numHarmonics * sizeof(float));
		memset(ptr, 0.0f, (_numHarmonics * sizeof(float)));
		_timeConvolvedFftHistory.add(ptr);
	}

	for (int i = 0; i < _smoothedFftHistory.totalSize(); i++) { //allocate memory for smoothed history (init memory to 0)
		float* ptr = (float*)malloc(_numHarmonics * sizeof(float));
		memset(ptr, 0.0f, (_numHarmonics * sizeof(float)));
		_smoothedFftHistory.add(ptr);
	}
}


void FFTs::calculateFft(FourierTransformType type)
{
	if (type == FourierTransformType::STANDARD)
		calculateStandardFft();
	else if (type == FourierTransformType::TIME_CONVOLVED)
		calculateTimeConvolvedFft();
	else if (type == FourierTransformType::SMOOTHED)
		calculateSmoothedFft();
}

History<float*>* FFTs::getFftHistory(FourierTransformType type)
{
	if (type == FourierTransformType::STANDARD)
		return _fftHistoryPtr;
	else if (type == FourierTransformType::TIME_CONVOLVED)
		return &_timeConvolvedFftHistory;
	else if (type == FourierTransformType::SMOOTHED)
		return &_smoothedFftHistory;

	return nullptr;
};


void FFTs::convolveOverHarmonics(float* in, float* out, int windowSize, Kernel kernel)
{
	memset(out, 0.0f, _numHarmonics);

	for (int i = 0; i < _numHarmonics; i++) {
		int jStart = std::max(i - (windowSize / 2), 0);
		for (int j = jStart; j < std::min(i + (windowSize / 2), _numHarmonics); j++) {
			out[i] += in[j] * Kernels::apply(kernel, j - jStart, windowSize);
		}
	}
}

void FFTs::calculateStandardFft() {
	//make sure not called twice on same frame
	if (_sampleFftLastCalculated == _m->_currentSample) {
		return;
	}
	_sampleFftLastCalculated = _m->_currentSample;

	_fftwAPI.calculate(_m->_audioData, _m->_currentSample, 500); //use fftw api to calculate fft
	//updates _fftHistory ^^^
}


void FFTs::calculateTimeConvolvedFft(int previousXtransforms, Kernel kernel)
{
	if (_timeConvolvedFftHistory.newestSample() == _m->_currentSample) { return; } //already calculated this frame then return

	//sanity check input
	if (previousXtransforms > _fftHistoryPtr->totalSize()) {
		Vengine::warning("Requested to convolve over more frames than exists stored fft data, defaulting to over all stored data");
		previousXtransforms = _fftHistoryPtr->totalSize();
	}

	//dependencies
	calculateStandardFft(); //relies on basic fft

	memset(_timeConvolvedFftHistory.oldest(), 0.0f, _numHarmonics * sizeof(float));

	for (int i = 0; i < previousXtransforms; i++) {
		for (int j = 0; j < _numHarmonics; j++) {
			_timeConvolvedFftHistory.oldest()[j] += _fftHistoryPtr->get(i)[j] * Kernels::apply(LINEAR_PYRAMID, i, previousXtransforms); //overwrite oldest
		}
	}

	_timeConvolvedFftHistory.add(_timeConvolvedFftHistory.oldest(), _m->_currentSample); //add oldest, now newest
}

void FFTs::calculateSmoothedFft(bool useTimeConvolvedFft, float attack, float release, float maxAccelerationPerSecond)
{
	if (_m->_previousSample == -1) {
		return;
	}

	if (_timeConvolvedFftHistory.newestSample() == _m->_currentSample) { return; } //already calculated this frame then return

	//dependencies based on paramters
	float* fftToUse;
	if (!useTimeConvolvedFft) {
		calculateStandardFft();
		fftToUse = _fftHistoryPtr->newest();
	}
	else{
		calculateTimeConvolvedFft();
		fftToUse = _timeConvolvedFftHistory.newest();
	}


	float timeSinceLastCalculation = float(_m->_currentSample - _m->_previousSample) / float(_m->_sampleRate);
	float maxAcceleration = maxAccelerationPerSecond * timeSinceLastCalculation;

	for (int i = 0; i < _numHarmonics; i++) {
		if (fftToUse[i] > _smoothedFftHistory.newest()[i]) {
			float change = std::fmin(timeSinceLastCalculation / attack, fftToUse[i] - _smoothedFftHistory.newest()[i]);
			change = std::fmin(_smoothedFftDot[i] + maxAcceleration, change); //clamp change to be within max acceleration of last change
			_smoothedFftDot[i] = change;
		}
		else {
			float change = -std::fmin(timeSinceLastCalculation / release, _smoothedFftHistory.newest()[i] - fftToUse[i]);
			change = std::fmax(_smoothedFftDot[i] - maxAcceleration, change);
			_smoothedFftDot[i] = change;
		}
		_smoothedFftHistory.oldest()[i] = _smoothedFftHistory.newest()[i] + _smoothedFftDot[i];
		if (_smoothedFftHistory.oldest()[i] < 0) {
			_smoothedFftHistory.oldest()[i] = 0;
		}
	}

	_smoothedFftHistory.add(_smoothedFftHistory.oldest()); //add oldest, now newest
}
