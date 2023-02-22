#include "VisualiserShader.h"
#include <fstream>

static const std::string COMMON_PREFIX_PATH = "Resources/Shaders/fragPrefix.frag";
static std::vector<std::string> _commonPrefixSave;

static const std::string DEFAULT_VERT_PATH = "Resources/Shaders/default.vert";
static std::vector<std::string> _defaultVertSave;

static const std::string AUGMENTED_SHADER_FOLDER_NAME = ".preparedShaders";

void VisualiserShader::init(const std::string& shaderPath, const std::string& visualiserPath)
{
	updateShaderName(shaderPath);
	_sourcePath = shaderPath;

	Vengine::IOManager::readTextFileToBuffer(visualiserPath + "/shaders/" + _shaderName + ".frag", _shaderContents);

	addCommonPrefix();
	createVertFragPairAndSetProgram(visualiserPath);
}



//private


void VisualiserShader::addCommonPrefix()
{
	if (_commonPrefixSave.size() == 0) {
		Vengine::IOManager::readTextFileToBuffer(COMMON_PREFIX_PATH, _commonPrefixSave);
	}

	prefix(_commonPrefixSave);
}

void VisualiserShader::createVertFragPairAndSetProgram(const std::string& visualiserPath)
{
	std::string asfp = visualiserPath + "/" + AUGMENTED_SHADER_FOLDER_NAME; //augmented shader folder path
	Vengine::IOManager::createFolder(asfp, true);
	Vengine::IOManager::outputTextFile(asfp + "/" + _shaderName + ".frag", _shaderContents);

	if (_defaultVertSave.size() == 0) {
		Vengine::IOManager::readTextFileToBuffer(DEFAULT_VERT_PATH, _defaultVertSave);
	}

	Vengine::IOManager::outputTextFile(asfp + "/" + _shaderName + ".vert", _defaultVertSave);

	_program = Vengine::ResourceManager::getShaderProgram(asfp + "/" + _shaderName);
}

void VisualiserShader::prefix(std::vector<std::string>& lines)
{
	int sourceSize = _shaderContents.size();
	int linesSize = lines.size();
	_shaderContents.resize(sourceSize + linesSize);
	//shift everything right to make space for prefix
	for (int i = sourceSize + linesSize - 1; i >= linesSize; i--) {
		_shaderContents[i] = _shaderContents[i - linesSize];
	}
	//add prefix
	for (int i = 0; i < linesSize; i++) {
		_shaderContents[i] = lines[i];
	}
}

void VisualiserShader::suffix(std::vector<std::string>& lines)
{
	int sourceSize = _shaderContents.size();
	int linesSize = lines.size();
	_shaderContents.resize(sourceSize + linesSize);
	//add suffix
	for (int i = 0; i < linesSize; i++) {
		_shaderContents[i + sourceSize] = lines[i];
	}
}

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

