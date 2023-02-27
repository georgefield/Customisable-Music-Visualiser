#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"
#include <Vengine/IOManager.h>


std::unordered_map<std::string, VisualiserShader> VisualiserShaderManager::_shaderCache;
std::string VisualiserShaderManager::_currentVisualiserPath = "";
std::set<int> VisualiserShaderManager::_currentBindings;

//ssbo
std::unordered_map<int, SSBOinfo> VisualiserShaderManager::_SSBOinfoMap;
std::unordered_map<int, std::function<float* ()>> VisualiserShaderManager::_updaterFunctionMap;

//default fragment shader
static const std::string DEFAULT_FRAGMENT_SHADER_PATH = "Resources/shaders/simple.frag";

VisualiserShader* VisualiserShaderManager::getShader(std::string fragPath) {
	if (_shaderCache.find(fragPath) != _shaderCache.end()) {
		std::cout << "Shader " + fragPath + " already loaded" << std::endl;
		return &_shaderCache[fragPath];
	}

	//if shader from external add to internal shaders folder
	std::string newFragPath = fragPath;
	if (!Vengine::IOManager::isInParentDirectory(VisualiserManager::shadersFolder(), fragPath)) {
		newFragPath = VisualiserManager::shadersFolder() + fragPath.substr(fragPath.find_last_of('/')); //copy to shaders folder of visualiser
		Vengine::IOManager::copyFile(fragPath, newFragPath);
	}
	_shaderCache[newFragPath].init(newFragPath, _currentVisualiserPath);
	std::cout << "Shader " + fragPath + " loaded for first time" << std::endl;

	return &_shaderCache[newFragPath];
}

std::string VisualiserShaderManager::getDefaultFragmentShaderPath()
{
	return DEFAULT_FRAGMENT_SHADER_PATH;
}

//*** SSBOs ***

void VisualiserShaderManager::updateDynamicSSBOs()
{
	for (auto& it : _updaterFunctionMap) {
		SSBOinfo info = _SSBOinfoMap[it.first];
		Vengine::DrawFunctions::updateSSBO(info.id, it.first, it.second(), info.dataLength * sizeof(float));
	}
}

void VisualiserShaderManager::updateShaderUniforms()
{
	for (auto& it : _shaderCache) {
		it.second.updateUniformValues();
	}
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

	if (!SSBOalreadyBound(bindingId)) {
		Vengine::warning("Cannot delete SSBO with id " + std::to_string(bindingId) + " as it does not exist");
		return;
	}

	_SSBOinfoMap.erase(bindingId);
}

void VisualiserShaderManager::changeUpdaterFunctionForDynamicSSBO(int bindingId, std::function<float* ()> newUpdater)
{
	if (_updaterFunctionMap.find(bindingId) == _updaterFunctionMap.end()) {
		Vengine::warning("No current updater function found for dynamic SSBO " + std::to_string(bindingId));
		return;
	}

	_updaterFunctionMap[bindingId] = newUpdater;
}

bool VisualiserShaderManager::SSBOalreadyBound(int bindingId)
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

//***