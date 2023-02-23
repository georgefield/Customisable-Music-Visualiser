#include "FourierTransformManager.h"
#include "VisualiserShaderManager.h"

#include <functional>

std::unordered_map<int, int> FourierTransformManager::_SSBObindings;
std::unordered_map<int, FourierTransform*> FourierTransformManager::_fourierTransforms;
Master* FourierTransformManager::_master = nullptr;

void FourierTransformManager::setMaster(Master* master)
{
	_master = master;
}


void FourierTransformManager::createFourierTransform(int& id, int historySize, float cutOffLow, float cutOffHigh, float cutoffSmoothFrac)
{
	//id generation
	id = _fourierTransforms.size();
	while (_fourierTransforms.find(id) != _fourierTransforms.end()) {
		id++;
	}

	_fourierTransforms[id] = new FourierTransform(historySize, cutOffLow, cutOffHigh, cutoffSmoothFrac);
	_fourierTransforms[id]->init(_master);
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

	if (_SSBObindings.find(id) != _SSBObindings.end()) { //remove attached ssbo
		VisualiserShaderManager::eraseSSBO(_SSBObindings[id]);
		_SSBObindings.erase(id);
	}
}

FourierTransform* FourierTransformManager::getFourierTransform(int id)
{
	if (!fourierTransformExists(id)) {
		Vengine::warning("No fourier transform created with binding " + std::to_string(id));
	}

	return _fourierTransforms[id];
}

bool FourierTransformManager::bindOutputToSSBO(int id, int bindingId)
{
	//if already bound to ssbo
	if (_SSBObindings.find(id) != _SSBObindings.end()) {
		Vengine::warning("Unbinding fourier transform from old SSBO " + std::to_string(_SSBObindings[id]));

		//delete ssbo in visualiser shader manager
		VisualiserShaderManager::eraseSSBO(_SSBObindings[id]);
	}

	//if something else bound to ssbo
	if (VisualiserShaderManager::SSBOexists(bindingId)) {
		Vengine::warning("SSBO " + std::to_string(bindingId) + " already bound to something else. Replacing...");

		//remove from ssbo bindings
		for (auto& it : _SSBObindings) {
			if (it.second == bindingId) {
				_SSBObindings.erase(it.first);
				break;
			}
		}
		//delete ssbo in visualiser shader manager
		VisualiserShaderManager::eraseSSBO(bindingId);
	}



	std::function<float* ()> memberFuncToPass = std::bind(&FourierTransform::getOutput, _fourierTransforms[id]);
	_SSBObindings[id] = bindingId;

	return VisualiserShaderManager::initDynamicSSBO(bindingId, memberFuncToPass, _fourierTransforms[id]->getNumHarmonics());
}

std::string FourierTransformManager::SSBObindingStatus(int id)
{
	if (_SSBObindings.find(id) != _SSBObindings.end()) {
		return "Current binding = " + std::to_string(_SSBObindings[id]);
	}
	else {
		return "Not bound to SSBO";
	}
}

std::vector<int> FourierTransformManager::idArr()
{
	std::vector<int> ret;
	for (auto& it : _fourierTransforms) {
		ret.push_back(it.first);
	}
	return ret;
}
