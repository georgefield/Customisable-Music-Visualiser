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
	std::vector<std::string>* uniformNames = _program->getUniformNames();
	for (auto& it : *uniformNames) {

		if (getUniformType(it) == GL_FLOAT) {
			_setableUniformNames.push_back(it);
		}
		else if (getUniformType(it) == GL_INT) {
			_setableUniformNames.push_back(it);
		}
		else if (getUniformType(it) == GL_SAMPLER_2D) {
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
	_setIntUniformMap[uniformName] = setter;
	return true;
}

bool VisualiserShader::initSetterUniformPair(std::string uniformName, UniformSetter<float> setter)
{
	if (!uniformExists(uniformName, GL_FLOAT)) {
		Vengine::warning("Float uniform '" + uniformName + "' does not exist in shader '" + _shaderName + "'");
		return false;
	}

	if (!setter.isValid()) { return false; }
	_setFloatUniformMap[uniformName] = setter;
	return true;
}

void VisualiserShader::eraseSetterUniformPair(std::string uniformName)
{
	if (!uniformExists(uniformName)) {
		Vengine::warning("Cannot unset as uniform doesnt exist");
		return;
	}

	if (!uniformIsSet(uniformName)) {
		Vengine::warning("Cannot unset as uniform currently unset");
		return;
	}

	if (getUniformType(uniformName) == GL_FLOAT) {
		std::cout << "Erasing from float map" << std::endl;
		_setFloatUniformMap.erase(uniformName);
		return;
	}

	if (getUniformType(uniformName) == GL_INT) {
		std::cout << "Erasing from int map" << std::endl;
		_setIntUniformMap.erase(uniformName);
		return;
	}

	Vengine::fatalError("Uniform '" + uniformName + "' exists, is set, but has type not GL_FLOAT or GL_INT");
}

void VisualiserShader::updateUniformValues()
{
	if (!_program->isBeingUsed()) {
		Vengine::warning("Cannot update uniforms of shader '" + getName() + "' while shader is not being used");
		return;
	}

	for (auto& it : _setableUniformNames) {
		// if uniform is set
		if (uniformIsSet(it) && getUniformType(it) == GL_FLOAT) {
			//handle floats
			auto setFloatUniformLoc = _setFloatUniformMap.find(it);
			if (setFloatUniformLoc == _setFloatUniformMap.end()) {
				Vengine::fatalError("Something bad has happened where isSet & getType functions are wrong");
			}
			if (!(*setFloatUniformLoc).second.isValid()) {
				Vengine::fatalError("Invalid float setter");
			}

			(*setFloatUniformLoc).second.callUpdater();

			glUniform1f(_program->getUniformLocation((*setFloatUniformLoc).first), (*setFloatUniformLoc).second.functionValue);
		}
		else if (uniformIsSet(it) && getUniformType(it) == GL_INT) {
			//handle ints
			auto setIntUniformLoc = _setIntUniformMap.find(it);
			if (setIntUniformLoc == _setIntUniformMap.end()) {
				Vengine::fatalError("Something bad has happened where isSet & getType functions are wrong");
			}
			if (!(*setIntUniformLoc).second.isValid()) {
				Vengine::fatalError("Invalid int setter");
			}

			(*setIntUniformLoc).second.callUpdater();

			glUniform1i(_program->getUniformLocation((*setIntUniformLoc).first), (*setIntUniformLoc).second.functionValue);
		}

		// if uniform not set, set to 0
		else if (!uniformIsSet(it) && getUniformType(it) == GL_FLOAT) {
			glUniform1f(_program->getUniformLocation(it), 0.0f);
		}
		else if (!uniformIsSet(it) && getUniformType(it) == GL_INT) {
			glUniform1i(_program->getUniformLocation(it), 0);
		}
	}

	Vengine::testForGlErrors("Error updating shader uniforms in shader " + getName());
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


void VisualiserShader::getUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _setableUniformNames) {
		names.push_back(it);
	}
}

void VisualiserShader::getUnsetUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _setableUniformNames) {
		if (_setFloatUniformMap.find(it) == _setFloatUniformMap.end() && _setIntUniformMap.find(it) == _setIntUniformMap.end()) {
			names.push_back(it);
		}
	}
}

void VisualiserShader::getSetUniformNames(std::vector<std::string>& names)
{
	for (auto& it : _setFloatUniformMap) {
		names.push_back(it.first);
	}
	for (auto& it : _setIntUniformMap) {
		names.push_back(it.first);
	}
}

std::string VisualiserShader::getUniformSetterName(std::string uniformName)
{
	if (getUniformType(uniformName) == GL_INT) {
		return _setIntUniformMap[uniformName].setterName;
	}
	if (getUniformType(uniformName) == GL_FLOAT) {
		return _setFloatUniformMap[uniformName].setterName;
	}
	return std::string();
}

UniformSetter<float>* VisualiserShader::getFloatUniformSetterStruct(std::string uniformName)
{
	if (uniformIsSet(uniformName) && uniformExists(uniformName, GL_FLOAT)) {
		return &_setFloatUniformMap[uniformName];
	}
	Vengine::warning("No float uniform with name " + uniformName + " set in shader " + getName());
	return nullptr;
}

UniformSetter<int>* VisualiserShader::getIntUniformSetterStruct(std::string uniformName)
{
	if (uniformIsSet(uniformName) && uniformExists(uniformName, GL_INT)) {
		return &_setIntUniformMap[uniformName];
	}
	Vengine::warning("No int uniform with name " + uniformName + " set in shader " + getName());
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
	if (std::find(_setableUniformNames.begin(), _setableUniformNames.end(), name) != _setableUniformNames.end()) {
		if (type == NULL) { //NULL => any type
			return true;
		}

		if (getUniformType(name) == type) {
			return true;
		}
	}
	return false;
}

GLenum VisualiserShader::getUniformType(std::string name) //use instead of _program->getUniformType to help with debugging
{
	if (_program == nullptr) {
		Vengine::fatalError("Shader not initialised, cannot get a uniform type");
	}
	return _program->getUniformType(name);
}

bool VisualiserShader::uniformIsSet(std::string name)
{
	if (_setIntUniformMap.find(name) != _setIntUniformMap.end() ||
		_setFloatUniformMap.find(name) != _setFloatUniformMap.end()) {
		return true;
	}
	return false;
}

