#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <Vengine/Vengine.h>

#include "SignalProcessing.h"

struct Uniform {

	std::string name;
	void (*uniformSetterFunction)(GLuint location);
};

class UniformSetting
{
public:
	
	static void setUniforms(Vengine::GLSLProgram* shaderProgram);

	static void init(SignalProcessing* signalProcessor);

private:
	static bool _isInitialised;
	static std::vector<Uniform> _uniforms;

	static SignalProcessing* _signalProcessor;

	//uniform setter functions
	static void timeSetter(GLuint location);
	static void rmsSetter(GLuint location);
};

