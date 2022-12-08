#pragma once
#include <string>
#include <GL/glew.h>
#include <map>
#include <vector>

namespace Vengine {

	class GLSLProgram
	{
	public:
		GLSLProgram();
		~GLSLProgram();

		void compileShaders(const std::string& vertShaderFilepath, const std::string& fragShaderFilepath);
		void linkShaders();
		void addAttrib(const std::string& attribName);
		void updateUniformData();

		GLuint getUniformLocation(const std::string& uniformName);

		void use();
		void unuse();

		//getters
		std::vector<std::string>* getUniformNames() { return &(_uniformNames); }
		GLuint getID() const { return _programID; }
		GLenum getUniformType(std::string name);

	private:
		std::map<std::string, GLenum> _shaderUniforms;
		std::vector<std::string> _uniformNames;

		void compileShader(const std::string& filepath, GLuint& id);

		std::string _shaderName;

		GLuint _numAttribs;

		GLuint _programID;
		GLuint _vertexShaderID, _fragShaderID;
	};

}