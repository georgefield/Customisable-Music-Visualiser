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

	bool initSetterUniformPair(std::string uniformName, UniformSetter<int> setter);
	bool initSetterUniformPair(std::string uniformName, UniformSetter<float> setter);
	void unsetSetterUniformPair(std::string uniformName);

	void updateUniformValues();

	//info getters

	bool containsTextureUniform() { return _containsTextureUniform; }
	std::string getName() { return _shaderName; }
	std::string getSourcePath() { return _sourcePath; }
	std::string getSourceCode();

	void getFloatUniformNames(std::vector<std::string>& names);
	void getIntUniformNames(std::vector<std::string>& names);
	void getUnsetFloatUniformNames(std::vector<std::string>& names);
	void getUnsetIntUniformNames(std::vector<std::string>& names);
	void getSetFloatUniformNames(std::vector<std::string>& names);
	void getSetIntUniformNames(std::vector<std::string>& names);
	UniformSetter<float>* getFloatUniformSetterStruct(std::string uniformName);
	UniformSetter<int>* getIntUniformSetterStruct(std::string uniformName);

private:
	std::string _shaderName;
	std::string _sourcePath;

	bool _containsTextureUniform;

	Vengine::GLSLProgram* _program;

	void updateShaderName(std::string path);	//string manip to get name from given path, called when init called

		//uniform managing--
	//id for uniform maps is uniform name
	std::unordered_map<std::string, UniformSetter<float>> _uniformInfoMapFLOAT;
	std::unordered_map<std::string, UniformSetter<int>> _uniformInfoMapINT;

	bool uniformExists(std::string name, GLenum type);
};

