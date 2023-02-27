#pragma once
#include <Vengine/Vengine.h>
#include <string>
#include <vector>
#include <functional>

#include "UniformSetter.h"


class VisualiserShader
{
public:
	VisualiserShader() :
		_program(nullptr),
		_containsTextureUniform(false)
	{}

	void init(const std::string& fragPath, const std::string& visualiserPath);

	Vengine::GLSLProgram* getProgram() const { if (_program == nullptr) { Vengine::fatalError("Program not initialised"); } return _program; }

	void updateUniformValues();

	bool initSetterUniformPair(std::string uniformName, UniformSetter<int> setter);
	bool initSetterUniformPair(std::string uniformName, UniformSetter<float> setter);
	void eraseSetterUniformPair(std::string uniformName);


	//info getters

	bool containsTextureUniform() { return _containsTextureUniform; }
	std::string getName() { return _shaderName; }
	std::string getSourcePath() { return _sourcePath; }
	std::string getSourceCode();

	void getUniformNames(std::vector<std::string>& names);
	void getUnsetUniformNames(std::vector<std::string>& names);
	void getSetUniformNames(std::vector<std::string>& names);
	std::string getUniformSetterName(std::string uniformName);

	UniformSetter<float>* getFloatUniformSetterStruct(std::string uniformName);
	UniformSetter<int>* getIntUniformSetterStruct(std::string uniformName);

	GLenum getUniformType(std::string name);
private:
	std::string _shaderName;
	std::string _sourcePath;

	bool _containsTextureUniform;

	Vengine::GLSLProgram* _program;


	//uniform managing--
	std::vector<std::string> _setableUniformNames;

	//id for uniform maps is uniform name
	std::unordered_map<std::string, UniformSetter<float>> _setFloatUniformMap;
	std::unordered_map<std::string, UniformSetter<int>> _setIntUniformMap;

	void updateShaderName(std::string path); //string manip to get name from given path, called when init called
	bool uniformExists(std::string name, GLenum type = NULL);
	bool uniformIsSet(std::string name);
	//--
};

