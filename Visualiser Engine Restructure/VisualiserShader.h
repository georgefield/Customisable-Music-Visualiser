#pragma once
#include <Vengine/Vengine.h>
#include <string>
#include <vector>

class VisualiserShader
{
public:
	VisualiserShader() :
		_program(nullptr){}

	void init(const std::string& shaderPath, const std::string& visualiserPath);

	Vengine::GLSLProgram* getProgram() const { return _program; }

	std::string getName() { return _shaderName; }
	std::string getSourcePath() { return _sourcePath; }

private:
	std::string _shaderName;
	std::string _sourcePath;

	Vengine::GLSLProgram* _program;
	std::vector<std::string> _shaderContents;

	void addCommonPrefix();
	void createVertFragPairAndSetProgram(const std::string& visualiserPath);

	void prefix(std::vector<std::string>& lines);
	void suffix(std::vector<std::string>& lines);

	void updateShaderName(std::string path);	//string manip to get name from given path, called when init called
};

