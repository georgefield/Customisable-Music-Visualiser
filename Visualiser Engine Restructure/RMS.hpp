#pragma once
#include "Master.h"

class RMS {
public:

	RMS(int historySize) :
		_rms(historySize)
	{
	}

	void init(Master* master) {
		_m = master;
		_sampleLastCalculated = -1;
		_sumOfSamplesSquared = 0;
	}

	//void calculateNext(Kernel kernel);

	float getRMS() { return _rms.newest(); }
	History<float>* getHistory() {
		return &_rms;
	}

	void calculateNext(int windowSize, Kernel kernel) {

		//make sure not called twice on same frame
		if (_sampleLastCalculated == _m->_currentSample) {
			return;
		}
		_sampleLastCalculated = _m->_currentSample;

		//dependencies
		//none

		if (_sampleLastCalculated != -1 && _m->_previousSample != -1 && _m->_currentSample - _m->_previousSample < windowSize) { //save compute power if window only adjusted by a bit
			for (int i = _m->_previousSample; i < _m->_currentSample; i++) {
				_sumOfSamplesSquared -= _m->_audioData[i] * _m->_audioData[i];
			}

			for (int i = _m->_previousSample + windowSize; i < _m->_currentSample + windowSize; i++) {
				_sumOfSamplesSquared += _m->_audioData[i] * _m->_audioData[i];
			}
		}
		else {
			_sumOfSamplesSquared = 0;
			for (int i = _m->_currentSample; i < _m->_currentSample + windowSize; i++) {
				_sumOfSamplesSquared += _m->_audioData[i] * _m->_audioData[i];
			}
		}

		_rms.add(sqrtf(_sumOfSamplesSquared / windowSize), _m->_currentSample);
	}

private:
	Master* _m;

	History<float> _rms;
	float _sumOfSamplesSquared;

	int _sampleLastCalculated;
};