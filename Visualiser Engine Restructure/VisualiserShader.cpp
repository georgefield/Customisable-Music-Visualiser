#include "VisualiserShader.h"
#include <fstream>

static const std::string DEFAULT_VERT_PATH = "Resources/Shaders/default.vert";
static std::vector<std::string> _defaultVertSave;

void VisualiserShader::init(const std::string& fragPath, const std::string& visualiserPath)
{
	updateShaderName(fragPath);
	_sourcePath = fragPath;

	_program = Vengine::ResourceManager::getShaderProgram(DEFAULT_VERT_PATH, fragPath);

	//init maps
	std::vector<std::string> uniformNames = *(_program->getUniformNames());
	for (auto& it : uniformNames) {

		if (_program->getUniformType(it) == GL_FLOAT) {
			_uniformInfoMapFLOAT[it].initialiseAsNotSet();
		}
		else if (_program->getUniformType(it) == GL_INT) {
			_uniformInfoMapINT[it].initialiseAsNotSet();
		}
		else if (_program->getUniformType(it) == GL_SAMPLER_2D) {
			if (!_containsTextureUniform) {
				_containsTextureUniform = true;
			}
			else {
				Vengine::warning("More than one sampler2D detected in " + fragPath + ", 2nd sampler 2D (" + it + ") cannot be used");
			}
		}
		else {
			Vengine::warning("Uniform detected in " + fragPath + " with type not equal to int, float, or sampler2D");
		}
	}

}

bool VisualiserShader::initSetterUniformPair(std::string uniformName, UniformSetter<int> setter)
{
	if (!uniformExists(uniformName, GL_INT)) {
		Vengine::warning("Integer uniform '" + uniformName + "' does not exist in shader '" + _shaderName + "'");
		return false;
	}

	if (!setter.isValid()) { return false; }
	_uniformInfoMapINT[uniformName] = setter;
	return true;
}

bool VisualiserShader::initSetterUniformPair(std::string uniformName, UniformSetter<float> setter)
{
	if (!uniformExists(uniformName, GL_FLOAT)) {
		Vengine::warning("Float uniform '" + uniformName + "' does not exist in shader '" + _shaderName + "'");
		return false;
	}

	if (!setter.isValid()) { return false; }
	_uniformInfoMapFLOAT[uniformName] = setter;
	return true;
}

void VisualiserShader::unsetSetterUniformPair(std::string uniformName)
{
	if (_uniformInfoMapFLOAT.find(uniformName) != _uniformInfoMapFLOAT.end()) {
		if (!_uniformInfoMapFLOAT[uniformName].isSet) {
			Vengine::warning("Cannot unset as uniform currently unset");
			return;
		}
		_uniformInfoMapFLOAT[uniformName].initialiseAsNotSet();
		return;
	}

	if (_uniformInfoMapINT.find(uniformName) != _uniformInfoMapINT.end()) {
		if (!_uniformInfoMapINT[uniformName].isSet) {
			Vengine::warning("Cannot unset as uniform currently unset");
			return;
		}
		_uniformInfoMapINT[uniformName].initialiseAsNotSet();
		return;
	}

	Vengine::warning("Uniform '" + uniformName + "' does not exist in shader so cannot unset it");
	return;
}

void VisualiserShader::updateUniformValues()
{
	//update floats
	for (auto& it : _uniformInfoMapFLOAT) {
		if (!it.second.isValid()) {
			Vengine::fatalError("Invalid float setter");
		}
		else if (it.second.isConstant) {
			//set constant value if constant attached
			glUniform1f(_program->getUniformLocation(it.first), it.second.functionValue);
		}
		else if (it.second.functionIsAttached) {
			it.second.callUpdater();
			glUniform1f(_program->getUniformLocation(it.first), it.second.functionValue);
		}

		Vengine::testForGlErrors("Error updating shader uniform " + it.first + " in shader " + getName());
	}

	//update ints
	for (auto& it : _uniformInfoMapINT) {
		if (!it.second.isValid()) {
			Vengine::fatalError("Invalid int setter");
		}
		else if (it.second.isConstant) {
			//set constant value if constant attached
			glUniform1i(_program->getUniformLocation(it.first), it.second.functionValue);
		}
		else if (it.second.functionIsAttached) {
			it.second.callUpdater();
			glUniform1i(_program->getUniformLocation(it.first), it.second.functionValue);
		}

		Vengine::testForGlErrors("Error updating shader uniform " + it.first + " in shader " + getName());
	}

}

std::string VisualiserShader::getSourceCode()
{
	std::vector<std::string> tmp;
	Vengine::IOManager::readTextFileToBuffer(getSourcePath(), tmp);
	std::string out;
	for (auto& it : tmp) {
		out += it + "\n";
	}
	return out;
}

void VisualiserShader::getFloatUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _uniformInfoMapFLOAT) {
		names.push_back(it.first);
	}
}

void VisualiserShader::getIntUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _uniformInfoMapINT) {
		names.push_back(it.first);
	}
}

void VisualiserShader::getUnsetFloatUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _uniformInfoMapFLOAT) {
		if (!it.second.isSet) {
			names.push_back(it.first);
		}
	}
}

void VisualiserShader::getUnsetIntUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _uniformInfoMapINT) {
		if (!it.second.isSet) {
			names.push_back(it.first);
		}
	}
}

void VisualiserShader::getSetFloatUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _uniformInfoMapFLOAT) {
		if (it.second.isSet) {
			names.push_back(it.first);
		}
	}
}

void VisualiserShader::getSetIntUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _uniformInfoMapINT) {
		if (it.second.isSet) {
			names.push_back(it.first);
		}
	}
}

UniformSetter<float>* VisualiserShader::getFloatUniformSetterStruct(std::string uniformName)
{
	if (uniformExists(uniformName, GL_FLOAT)) {
		return &_uniformInfoMapFLOAT[uniformName];
	}
	Vengine::warning("No float uniform with name " + uniformName + " in shader " + getName());
	return nullptr;
}

UniformSetter<int>* VisualiserShader::getIntUniformSetterStruct(std::string uniformName)
{
	if (uniformExists(uniformName, GL_INT)) {
		return &_uniformInfoMapINT[uniformName];
	}
	Vengine::warning("No int uniform with name " + uniformName + " in shader " + getName());
	return nullptr;
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

bool VisualiserShader::uniformExists(std::string name, GLenum type)
{
	if (type == GL_FLOAT) {
		if (_uniformInfoMapFLOAT.find(name) == _uniformInfoMapFLOAT.end()) {
			return false;
		}
		return true;
	}
	else if (type == GL_INT) {
		if (_uniformInfoMapINT.find(name) == _uniformInfoMapINT.end()) {
			return false;
		}
		return true;
	}
	else {
		return false;
	}
}

