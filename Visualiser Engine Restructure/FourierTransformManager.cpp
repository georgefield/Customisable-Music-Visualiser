#include "FourierTransformManager.h"
#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"
#include "SignalProcessingManager.h"
#include "UIglobalFeatures.h"
#include <functional>

std::unordered_map<int, FourierTransform*> FourierTransformManager::_fourierTransforms;


void FourierTransformManager::createFourierTransformFromStruct(FourierTransform::FTinfo info)
{
	createFourierTransform(info.id, info.historySize, info.cutoffLow, info.cutoffHigh, info.cutoffSmoothFrac);
	memcpy(&getFourierTransform(info.id)->_FTinfo, &info, sizeof(FourierTransform::FTinfo)); //copy info
}

void FourierTransformManager::createFourierTransform(int id, int historySize, float cutOffLow, float cutOffHigh, float cutoffSmoothFrac)
{
	if (id >= SP::consts._maxFourierTransforms) {
		Vengine::warning("id greater than max fourier transforms");
		return;
	}

	if (fourierTransformExists(id)) {
		Vengine::warning("Cannot create FT of id " + std::to_string(id) + " as it already exists");
		return;
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

void FourierTransformManager::clearFourierTransforms()
{
	for (auto& id : idArr()) {
		eraseFourierTransform(id);
	}
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
		it.second->calculateNext();
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

std::vector<int> FourierTransformManager::availiableIdArr()
{
	std::vector<int> availiableFTids;
	for (int i = 0; i < SP::consts._maxFourierTransforms; i++) {
		if (!fourierTransformExists(i)) {
			availiableFTids.push_back(i);
		}
	}
	return availiableFTids;
}
