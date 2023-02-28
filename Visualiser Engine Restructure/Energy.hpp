#pragma once
#include "Master.h"

class Energy {
public:

	Energy(int historySize):
		_energy(historySize)
	{}

	void init(Master* master) {
		_m = master;
		_sampleLastCalculated = -1;
	}

	//void calculateNext(Kernel kernel);

	float getEnergy() { return _energy.newest(); }
	History<float>* getHistory() {
		return &_energy;
	}

	void calculateNext(int convolveWindowSize, Kernel kernel) {

		//make sure not called twice on same frame
		if (_sampleLastCalculated == _m->_currentSample) {
			return;
		}
		_sampleLastCalculated = _m->_currentSample;

		//dependencies
		//none

		float sum = 0;
		for (int i = _m->_currentSample; i < _m->_currentSample + convolveWindowSize; i++) {
			sum += _m->_audioData[i] * _m->_audioData[i] * Kernels::apply(kernel, i - _m->_currentSample, convolveWindowSize);
		}
		_energy.add(sum);
	}

private:
	Master* _m;

	History<float> _energy;

	int _sampleLastCalculated;
};