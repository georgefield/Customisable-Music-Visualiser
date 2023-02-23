#include "VisualiserShaderManager.h"
#include <Vengine/IOManager.h>


std::unordered_map<int, VisualiserShader> VisualiserShaderManager::_shaderCache;
std::string VisualiserShaderManager::_currentVisualiserPath = "";
std::set<int> VisualiserShaderManager::_currentBindings;

std::unordered_map<int, SSBOinfo> VisualiserShaderManager::_SSBOinfoMap;
std::unordered_map<int, std::function<float* ()>> VisualiserShaderManager::_updaterFunctionMap;


void VisualiserShaderManager::add(std::string shaderPath, int& id)
{
	if (_currentVisualiserPath == "") {
		Vengine::warning("No visualiser loaded, cannot add shader");
		return;
	}

	//id generation
	id = _shaderCache.size();
	while (_shaderCache.find(id) != _shaderCache.end()) {
		id++;
	}

	_shaderCache[id].init(shaderPath, _currentVisualiserPath);
}

void VisualiserShaderManager::updateDynamicSSBOs()
{
	for (auto& it : _updaterFunctionMap) {
		SSBOinfo info = _SSBOinfoMap[it.first];
		Vengine::DrawFunctions::updateSSBO(info.id, it.first, it.second(), info.dataLength * sizeof(float));
	}
}

void VisualiserShaderManager::updateAugmentedShaders()
{
	//todo
}

bool VisualiserShaderManager::initStaticSSBO(int bindingId, float* staticData, int dataLength)
{
	if (!SSBOinitPossible()) {
		return false;
	}

	if (_SSBOinfoMap.find(bindingId) != _SSBOinfoMap.end()) {
		Vengine::warning("Binding id " + std::to_string(bindingId) + " is not availiable, earliest avialiable = " + std::to_string(getNextAvailiableBinding()));
		return false;
	}

	SSBOinfo s;
	s.data = staticData;
	s.dataLength = dataLength;

	Vengine::DrawFunctions::createSSBO(s.id, bindingId, s.data, s.dataLength * sizeof(float), GL_STATIC_COPY);
	_SSBOinfoMap[bindingId] = s;

	return true;
}

bool VisualiserShaderManager::initDynamicSSBO(int bindingId, std::function<float* ()> updaterFunction, int dataLength)
{
	if (!SSBOinitPossible()) {
		return false;
	}

	if (_SSBOinfoMap.find(bindingId) != _SSBOinfoMap.end()) {
		Vengine::warning("Binding id " + std::to_string(bindingId) + " is not availiable, earliest avialiable = " + std::to_string(getNextAvailiableBinding()));
		return false;
	}

	SSBOinfo s;
	s.data = updaterFunction(); //set data to be value returned from data getter
	s.dataLength = dataLength;

	Vengine::DrawFunctions::createSSBO(s.id, bindingId, s.data, s.dataLength * sizeof(float), GL_DYNAMIC_DRAW);
	_SSBOinfoMap[bindingId] = s;
	_updaterFunctionMap[bindingId] = updaterFunction;
	
	return true;
}

void VisualiserShaderManager::eraseSSBO(int bindingId)
{
	if (_updaterFunctionMap.find(bindingId) == _updaterFunctionMap.end()) {
		Vengine::warning("No updater function found for dynamic SSBO " + std::to_string(bindingId) + ", assumed SSBO is static");
	}
	else {
		_updaterFunctionMap.erase(bindingId);
		std::cout << "Deleted updater function" << bindingId<< std::endl;
	}

	if (!SSBOexists(bindingId)) {
		Vengine::warning("Cannot delete SSBO with id " + std::to_string(bindingId) + " as it does not exist");
		return;
	}

	_SSBOinfoMap.erase(bindingId);
	updateAugmentedShaders();
}

void VisualiserShaderManager::changeUpdaterFunctionForDynamicSSBO(int bindingId, std::function<float* ()> newUpdater)
{
	if (_updaterFunctionMap.find(bindingId) == _updaterFunctionMap.end()) {
		Vengine::warning("No current updater function found for dynamic SSBO " + std::to_string(bindingId));
		return;
	}

	_updaterFunctionMap[bindingId] = newUpdater;
}

bool VisualiserShaderManager::SSBOexists(int bindingId)
{
	return (_SSBOinfoMap.find(bindingId) != _SSBOinfoMap.end());
}

bool VisualiserShaderManager::SSBOinitPossible()
{
	//add checks using gl functions that there is enough space in memory
	return true;
}

int VisualiserShaderManager::getNextAvailiableBinding()
{
	//get smallest binding not being used
	int smallestUnusedBinding = 0; //initialize the smallest missing positive integer to 0

	for (int binding : _currentBindings) {
		if (binding == smallestUnusedBinding) {
			smallestUnusedBinding++; //increment smallestUnusedBinding if it's already in the set
		}
		else if (binding > smallestUnusedBinding) {
			return smallestUnusedBinding; //return when gap found
		}
	}
	//if all the elements in the set are consecutive, smallest is set to next integer after

	return smallestUnusedBinding;
}
