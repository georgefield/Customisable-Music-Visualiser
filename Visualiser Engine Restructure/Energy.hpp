#pragma once
#include "Master.h"

#include "VisualiserShaderManager.h"
#include <functional>

class Energy {
public:

	Energy(int historySize):
		_energy(historySize),
		_initialised(false),
		_useSetters(false)
	{}

	~Energy() {
		if (_useSetters) {
			deleteSetters();
		}
	}

	//only calculate energy from fourier transform to keep consistent as if allow from audio data have to deal with accounting for hanning window
	//and the frequencies removed when only including half fourier transform
	void init(Master* m, std::string nameOfFT, bool useSetters = true) {
		if (_initialised) {
			Vengine::warning("Please call reInit to restart already initialised energy class");
			return;
		}

		_nameOfFTtoBeAnalysed = nameOfFT;

		_m = m;
		_sampleLastCalculated = -1;
		_initialised = true;
		_useSetters = useSetters;

		if (_useSetters) {
			initSetters();
		}
	}

	void reInit() {
		if (!_initialised) {
			Vengine::warning("Did not call init before calling reInit for energy class");
			return;
		}

		_energy.clear();
		_sampleLastCalculated = -1;
	}

	//void calculateNext(Kernel kernel);

	float getEnergy() { return _energy.newest(); }
	History<float>* getHistory() {
		return &_energy;
	}

	void calculateNext(float* fourierTransform, int numHarmonics) {

		//make sure not called twice on same frame
		if (_sampleLastCalculated == _m->_currentSample) {
			return;
		}
		_sampleLastCalculated = _m->_currentSample;

		//dependencies
		//none

		float energy = 0.0f;
		for (int i = 0; i < numHarmonics; i++) {
			energy += fourierTransform[i] * fourierTransform[i];
		}
		energy /= float(_m->_fftHistory.numHarmonics()); //energy per sample, num harmonics = window size / 2, would divide by window size if working with audio data but because we delete half of fourier transform we divide by num harmonics
		energy *= float(_m->_sampleRate); //energy per second
		//although this not accurate anyway as we applied gain to fourier transform but its okay as only calculate from fourier transform. Bother doing this to sync between fourier values and energy
		_energy.add(energy, _m->_currentSample);
	}

private:
	Master* _m;

	History<float> _energy;
	std::string _nameOfFTtoBeAnalysed;

	bool _initialised;
	bool _useSetters;

	int _sampleLastCalculated;

	void initSetters() {
		VisualiserShaderManager::addHistoryAsPossibleSSBOsetter(_nameOfFTtoBeAnalysed + " energy", &_energy);
	}

	void deleteSetters() {
		VisualiserShaderManager::deleteHistoryAsPossibleSSBOsetter(_nameOfFTtoBeAnalysed + " energy");
	}
};