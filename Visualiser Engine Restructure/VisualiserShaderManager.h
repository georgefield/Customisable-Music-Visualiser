#pragma once
#include "VisualiserShader.h"
#include <unordered_map>

class VisualiserShaderManager
{
public:

	static void setCurrentVisualiserPath(const std::string& visualiserPath) { _currentVisualiserPath = visualiserPath; }
	static void add(std::string shaderPath, int& id);

private:

	static std::string _currentVisualiserPath;
	static std::unordered_map<int, VisualiserShader> _shaderCache;

};

