#include "VisualiserShader.h"
#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"
#include "VisVars.h"
#include <fstream>

void VisualiserShader::init(const std::string& fragPath, const std::string& visualiserPath)
{
	updateShaderName(fragPath);
	_sourcePath = fragPath;
	_visualiserPath = visualiserPath;

	_program = compile();
}


//private

void VisualiserShader::updateShaderName(std::string path)
{
	int substrStart = 0;

	// Find the last occurrence of the '/' character
	size_t lastSlash = path.find_last_of('/');
	if (lastSlash != std::string::npos) {
		substrStart = lastSlash;
	}

	// Get the substring after the last slash
	_shaderName = path.substr(substrStart + 1);

	// Find the first occurrence of the '.' character
	size_t dot = _shaderName.find_first_of('.');
	if (dot != std::string::npos) {
		// Get the substring before the dot
		_shaderName = _shaderName.substr(0, dot);
	}
}

Vengine::GLSLProgram* VisualiserShader::compile()
{
	std::string sourceCode;
	Vengine::IOManager::readTextFileToString(_sourcePath, sourceCode);

	sourceCode = VisualiserShaderManager::getCommonShaderPrefix() + sourceCode;

	_interpretedShaderSourcePath = VisualiserManager::interpretedShadersFolder()  + "/interpreted_" + _shaderName + ".frag";

	Vengine::IOManager::outputToTextFile(_interpretedShaderSourcePath, sourceCode, true);

	return Vengine::ResourceManager::getShaderProgram(VisVars::_commonVertShaderPath, _interpretedShaderSourcePath);
}
