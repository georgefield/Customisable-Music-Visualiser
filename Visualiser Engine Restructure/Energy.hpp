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

	void calculateNext(Kernel kernel) {

		//make sure not called twice on same frame
		if (_sampleLastCalculated == _m->_currentSample) {
			return;
		}
		_sampleLastCalculated = _m->_currentSample;

		//dependencies
		//none

		float sum = 0;
		for (int i = _m->_currentSample; i < _m->_currentSample + _m->_N; i++) {
			sum += fabsf(_m->_audioData[i]) * Kernels::apply(kernel, i - _m->_currentSample, _m->_N);
		}
		_energy.add(sum / _m->_N);
	}

private:
	Master* _m;

	History<float> _energy;

	int _sampleLastCalculated;
};