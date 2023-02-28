#include "FourierTransformManager.h"
#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"
#include "SignalProcessingManager.h"

#include <functional>

std::unordered_map<int, FourierTransform*> FourierTransformManager::_fourierTransforms;


void FourierTransformManager::createFourierTransform(int& id, int historySize, float cutOffLow, float cutOffHigh, float cutoffSmoothFrac)
{
	//id generation
	id = _fourierTransforms.size();
	while (_fourierTransforms.find(id) != _fourierTransforms.end()) {
		id++;
	}

	_fourierTransforms[id] = new FourierTransform(historySize, cutOffLow, cutOffHigh, cutoffSmoothFrac);
	_fourierTransforms[id]->init(SignalProcessing::getMasterPtr());

	addUniformSetterFunctionOptionsToList(id);
	addSSBOsetterFunctionOptionsToList(id);
}

bool FourierTransformManager::fourierTransformExists(int id)
{
	return (_fourierTransforms.find(id) != _fourierTransforms.end()); //true if exists, false if not
}

void FourierTransformManager::eraseFourierTransform(int id)
{
	if (!fourierTransformExists(id)) {
		Vengine::warning("No fourier transform created with binding " + std::to_string(id));
	}

	delete _fourierTransforms[id];
	_fourierTransforms.erase(id);

	//if ssbo set with unset itself when it gets a bad function call as _fourierTransforms[id] destroyed

	deleteUniformSetterFunctionOptionsFromList(id);
	deleteSSBOsetterFunctionOptionsFromList(id);
}

void FourierTransformManager::calculateFourierTransforms()
{
	//calculate the fourier transforms in fourier transform manager
	for (auto& it : _fourierTransforms){
		if (it.second->getMasterPtr() != SignalProcessing::getMasterPtr()) {
			Vengine::warning("Master pointer change, switch in song assumed, reiniting fourier transform");
			it.second->reInit(SignalProcessing::getMasterPtr());
		}
		it.second->beginCalculation();
		//test->applyFunction(FourierTransform::SMOOTH);
		//test->applyFunction(FourierTransform::FREQUENCY_CONVOLVE);
		it.second->endCalculation();
	}
}

FourierTransform* FourierTransformManager::getFourierTransform(int id)
{
	if (!fourierTransformExists(id)) {
		Vengine::warning("No fourier transform created with binding " + std::to_string(id));
	}

	return _fourierTransforms[id];
}

std::vector<int> FourierTransformManager::idArr()
{
	std::vector<int> ret;
	for (auto& it : _fourierTransforms) {
		ret.push_back(it.first);
	}
	return ret;
}

void FourierTransformManager::addUniformSetterFunctionOptionsToList(int id)
{
	std::string namePrefix = "FT-" + std::to_string(id);

	VisualiserShaderManager::Uniforms::addPossibleUniformSetter(namePrefix + " num harmonics", _fourierTransforms[id]->getNumHarmonics());
}

void FourierTransformManager::addSSBOsetterFunctionOptionsToList(int id)
{
	std::string functionName = "FT-" + std::to_string(id) + " harmonic values";
	std::function<float*()> memberFuncToPass = std::bind(&FourierTransform::getOutput, _fourierTransforms[id]);

	VisualiserShaderManager::SSBOs::addPossibleSSBOSetter(functionName, memberFuncToPass, _fourierTransforms[id]->getNumHarmonics());
}

void FourierTransformManager::deleteUniformSetterFunctionOptionsFromList(int id)
{
	std::string namePrefix = "FT-" + std::to_string(id);

	VisualiserShaderManager::Uniforms::deletePossibleUniformSetter(namePrefix + " num harmonics");
}

void FourierTransformManager::deleteSSBOsetterFunctionOptionsFromList(int id)
{
	std::string functionName = "FT-" + std::to_string(id) + " harmonic values";

	VisualiserShaderManager::SSBOs::deleteSSBOsetter(functionName);
}
