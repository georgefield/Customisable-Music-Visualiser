#pragma once
#include <string>
#include <GL/glew.h>

namespace Vengine {

	class GLSLProgram
	{
	public:
		GLSLProgram();
		~GLSLProgram();

		void compileShaders(const std::string& vertShaderFilepath, const std::string& fragShaderFilepath);
		void linkShaders();
		void addAttrib(const std::string& attribName);

		GLuint getUniformLocation(const std::string& uniformName);

		void use();
		void unuse();

	private:
		void compileShader(const std::string& filepath, GLuint& id);

		std::string _shaderName;

		GLuint _numAttribs;

		GLuint _programID;
		GLuint _vertexShaderID, _fragShaderID;
	};

}