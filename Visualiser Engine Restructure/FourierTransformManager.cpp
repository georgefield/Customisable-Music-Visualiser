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
	_fourierTransforms[id]->init(SignalProcessingManager::getMasterPtr(), id);
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
}

void FourierTransformManager::reInitAll()
{
	for (auto& it : _fourierTransforms) {
		it.second->reInit();
	}
}

void FourierTransformManager::calculateFourierTransforms()
{
	//calculate the fourier transforms in fourier transform manager
	for (auto& it : _fourierTransforms){
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