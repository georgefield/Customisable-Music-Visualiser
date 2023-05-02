#pragma once
#include "Master.h"

#include "VisualiserShaderManager.h"
#include "VisVars.h"
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
			removeUpdaters();
		}
	}


	//FTid = -1 when no info to be used for shaders
	void init(Master* m, int FTid) {
		if (_initialised) {
			Vengine::warning("Please call reInit to restart already initialised energy class");
			return;
		}

		_FTsourceId = FTid;

		_m = m;
		_sampleLastCalculated = -1;
		_initialised = true;
		_useSetters = (_FTsourceId >= 0);

		if (_useSetters) {
			initUpdaters();
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
		energy /= Vis::vars._masterFTgain * Vis::vars._masterFTgain; //fix the gain applied on transform
		energy *= 2; // miss out the mirrored half of the fourier transform
		
		_energy.add(energy);
	}

private:
	Master* _m;

	History<float> _energy;
	int _FTsourceId;

	bool _initialised;
	bool _useSetters;

	int _sampleLastCalculated;

	void initUpdaters() {
		std::string uniformName = "vis_FT" + std::to_string(_FTsourceId) + "_energy";
		std::function<float()> energyUpdaterFunction = std::bind(&Energy::getEnergy, this);
		VisualiserShaderManager::Uniforms::setUniformUpdater(uniformName, energyUpdaterFunction);
	}

	void removeUpdaters() {
		std::string uniformName = "vis_FT" + std::to_string(_FTsourceId) + "_energy";
		VisualiserShaderManager::Uniforms::removeUniformUpdater(uniformName);
	}
};