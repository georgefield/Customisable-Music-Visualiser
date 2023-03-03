#pragma once
#include "Master.h"
#include "VisualiserShaderManager.h"

#include <functional>

class RMS {
public:

	RMS(int historySize) :
		_rms(historySize)
	{
	}

	~RMS() {
		deleteSetters();
	}

	void init(Master* master) {
		_m = master;
		_sampleLastCalculated = -1;
		_sumOfSamplesSquared = 0;

		initSetters();
	}

	void reInit() {
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

		//saves compute power by just removing a old samples and adding new ones to _sumOfSamplesSquared. Only works if currentSample - previousSample < windowSize
		if (_sampleLastCalculated != -1 && _m->_previousSample != -1 && _m->_currentSample - _m->_previousSample < windowSize) { 

			for (int i = _m->_previousSample; i < _m->_currentSample; i++) {
				_sumOfSamplesSquared -= _m->_audioData[i] * _m->_audioData[i];
			}

			for (int i = _m->_previousSample + windowSize; i < _m->_currentSample + windowSize; i++) {
				_sumOfSamplesSquared += _m->_audioData[i] * _m->_audioData[i];
			}
		}
		//basic rms if cannot optimise
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

	void initSetters() {
		VisualiserShaderManager::addHistoryAsPossibleSSBOsetter("RMS", &_rms);
	}
	
	void deleteSetters() {
		VisualiserShaderManager::deleteHistoryAsPossibleSSBOsetter("RMS");
	}
};