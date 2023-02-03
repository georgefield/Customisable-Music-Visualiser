#include "MyFuncs.h"

#include <Vengine/MyErrors.h>

float MyFuncs::_rms = 0;


void MyFuncs::setUniformsForShader(Vengine::GLSLProgram* shaderProgram) {

	//set time to timer time
	for (auto& it : *(shaderProgram->getUniformNames())) {

		if (it == "time" && shaderProgram->getUniformType(it) == GL_FLOAT) {
			GLuint loc = shaderProgram->getUniformLocation(it);
			glUniform1f(loc, Vengine::MyTiming::readTimer(0));
		}
		else if (it == "rms" && shaderProgram->getUniformType(it) == GL_FLOAT) {
			GLuint loc = shaderProgram->getUniformLocation(it);
			glUniform1f(loc, _rms);
		}
	}
}


void MyFuncs::updateRMS(float rms) {
	_rms = rms;
}