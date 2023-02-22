#include "VisualiserShaderManager.h"
#include <Vengine/IOManager.h>


std::unordered_map<int, VisualiserShader> VisualiserShaderManager::_shaderCache;
std::string VisualiserShaderManager::_currentVisualiserPath = "";


void VisualiserShaderManager::add(std::string shaderPath, int& id)
{
	if (_currentVisualiserPath == "") {
		Vengine::warning("Shader not added. Please set current visualiser path first for visualiser shader manager.");
		return;
	}

	//id generation
	id = _shaderCache.size();
	while (_shaderCache.find(id) != _shaderCache.end()) {
		id++;
	}

	_shaderCache[id].init(shaderPath, _currentVisualiserPath);
}
