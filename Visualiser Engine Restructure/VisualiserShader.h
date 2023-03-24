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

	bool init(const std::string& fragPath, const std::string& visualiserPath);

	Vengine::GLSLProgram* getProgram() const { if (_program == nullptr) { Vengine::fatalError("Program not initialised"); } return _program; }

	//info getters

	std::string getName() { return _shaderName; }
	std::string getSourcePath() { return _sourcePath; }
	 
private:
	Vengine::GLSLProgram* compile();

	std::string _shaderName;
	std::string _sourcePath;
	std::string _interpretedShaderSourcePath;
	std::string _visualiserPath;

	bool _containsTextureUniform;

	Vengine::GLSLProgram* _program;

	void updateShaderName(std::string path); //string manip to get name from given path, called when init called

	void fixErrorMessage(std::string& errorMsg);
};

