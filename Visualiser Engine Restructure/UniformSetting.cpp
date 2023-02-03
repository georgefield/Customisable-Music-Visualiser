#include "UniformSetting.h"

std::vector<Uniform> UniformSetting::_uniforms;
bool UniformSetting::_isInitialised = false;

SignalProcessing* UniformSetting::_signalProcessor = nullptr;

void UniformSetting::setUniforms(Vengine::GLSLProgram* shaderProgram)
{
	if (!_isInitialised) {
		Vengine::fatalError("Cannot set uniforms as UniformSetting class not initialised");
	}

	std::vector<std::string> uniformNames = *(shaderProgram->getUniformNames());
	for (int i = 0; i < _uniforms.size(); i++) {
		if (std::find(uniformNames.begin(), uniformNames.end(), _uniforms[i].name) != uniformNames.end()) {
			_uniforms[i].uniformSetterFunction(shaderProgram->getUniformLocation(_uniforms[i].name));
		}
	}
}

void UniformSetting::init(SignalProcessing* signalProcessor)
{
	_isInitialised = true;
	_signalProcessor = signalProcessor;

	_uniforms.push_back({ "_time", timeSetter });
	_uniforms.push_back({ "_rms", rmsSetter });
}


//uniform setter functions
void UniformSetting::timeSetter(GLuint location) {
	glUniform1f(location, Vengine::MyTiming::readTimer(0));
}

void UniformSetting::rmsSetter(GLuint location) {
	glUniform1f(location, _signalProcessor->_rms.getRMS());
}