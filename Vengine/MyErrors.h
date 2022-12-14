#pragma once
#include <string>
#include <GL/glew.h>

namespace Vengine {

	extern void fatalError(std::string errorMessage);
	extern void warning(std::string warningMessage, bool pause = false);
	extern void logGLerror(std::string message, GLenum error);
}