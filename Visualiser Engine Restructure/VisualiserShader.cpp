#include "VisualiserShader.h"
#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"
#include "VisVars.h"
#include "UIglobalFeatures.h"
#include <fstream>

bool VisualiserShader::init(const std::string& fragPath, const std::string& visualiserPath)
{
	updateShaderName(fragPath);
	_sourcePath = fragPath;
	_visualiserPath = visualiserPath;

	_program = compile();
	if (_program == nullptr)
		return false;
	return true;
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

//vvv lots of string manip
void VisualiserShader::fixErrorMessage(std::string& errorMsg)
{
	std::string errorMsgSave = errorMsg; //save in case of failure

	//get message by line in order--
	std::vector<std::string> msgLineByLine;
	int nextNL = errorMsg.find_first_of('\n');
	while (nextNL != -1) {
		if (nextNL != 0)
			msgLineByLine.push_back(errorMsg.substr(0, nextNL)); //get error message by line
		errorMsg = errorMsg.substr(nextNL + 1);
		nextNL = errorMsg.find_first_of('\n');
	}
	if (errorMsg.size()	 != 0)
		msgLineByLine.push_back(errorMsg); //get final line
	//--

	//change error line numbers--
	int count = 0;
	for (auto& it : msgLineByLine) {
		if (count == 0) {
			count++;
			continue;
		}

		std::string errorLineNum = it;
		std::string prefix;
		std::string postfix;
		prefix = errorLineNum.substr(0, errorLineNum.find_first_of(":"));
		errorLineNum = errorLineNum.substr(errorLineNum.find_first_of(":") + 1); //after first 2 colons
		prefix += errorLineNum.substr(0, errorLineNum.find_first_of(":"));
		prefix += ":";
		errorLineNum = errorLineNum.substr(errorLineNum.find_first_of(":") + 1);
		postfix = ":";
		postfix += errorLineNum.substr(errorLineNum.find_first_of(":") + 1);
		errorLineNum = errorLineNum.substr(0, errorLineNum.find_first_of(":")); //before next colon

		int errorLineNumI = -1;
		try {
			errorLineNumI = std::stoi(errorLineNum);
			errorLineNumI -= 103;//number of lines in shader prefix
		}
		catch(std::exception e){
			Vengine::warning("Shader error conversion failed: " + (std::string)e.what());
			errorMsg = errorMsgSave;
			return;
		}

		it = prefix + std::to_string(errorLineNumI) + postfix;

		count++;
	}
	//--

	//set error message
	errorMsg = _shaderName + ".visfrag\n";
	errorMsg += VisualiserManager::shadersFolder() + "/" + _shaderName + ".visfrag\n";

	count = 0;
	for (auto& it : msgLineByLine) {
		if (count == 0) {
			count++;
			continue;
		}

		errorMsg += it + "\n";
		count++;
	}
}

Vengine::GLSLProgram* VisualiserShader::compile()
{
	std::string sourceCode;
	Vengine::IOManager::readTextFileToString(_sourcePath, sourceCode);

	sourceCode = VisualiserShaderManager::getCommonShaderPrefix() + sourceCode;

	_interpretedShaderSourcePath = VisualiserManager::interpretedShadersFolder()  + "/interpreted_" + _shaderName + ".frag";

	Vengine::IOManager::outputToTextFile(_interpretedShaderSourcePath, sourceCode, true);

	std::string errorOut;
	Vengine::GLSLProgram* program = Vengine::ResourceManager::reloadShaderProgram(VisVars::_commonVertShaderPath, _interpretedShaderSourcePath, errorOut);
	if (program == nullptr) {
		fixErrorMessage(errorOut);
		UIglobalFeatures::addSyntaxErrorToWindow(errorOut);
	}
	return program;
}
