#pragma once
#include <GL/glew.h>
#include <map>
#include <string>

#include "GLSLProgram.h"

namespace Vengine {

	class DrawFunctions
	{
	public:
		static void clearCurrentBuffer();
		static void setDrawTarget(GLuint bufferID, int sizeX, int sizeY);
		static void createDrawBuffers(GLuint* bufferID, GLuint* textureID, int sizeX, int sizeY, int num);
		static void uploadTextureToShader(GLSLProgram program, GLuint& textureID, const std::string& texVariableName, GLenum tex = GL_TEXTURE0, int num = 0);
		static void createSSBO(GLuint& ssboID, GLint binding, void* data, int bytesOfData, GLenum usage);
		static void updateSSBO(GLuint& ssboID, GLint binding, void* data, int bytesOfData);
		static void updateSSBOpart(GLuint& ssboID, GLint binding, void* data, int offset, int bytesOfData);
	};

}