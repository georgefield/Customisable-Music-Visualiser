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

		bool compileShaders(const std::string& vertShaderFilepath, const std::string& fragShaderFilepath);
		bool linkShaders();
		void addAttrib(const std::string& attribName);
		void updateShaderUniformInfo();

		GLuint getUniformLocation(const std::string& uniformName);

		std::string getSyntaxError();

		void use();
		void unuse();

		//getters
		bool isBeingUsed() const { return _isBeingUsed; }

		std::vector<std::string>* getUniformNames() { return &(_uniformNames); }
		GLuint getID() const { return _programID; }
		GLenum getUniformType(std::string name);

	private:
		std::string _syntaxError;

		std::map<std::string, GLenum> _shaderUniforms;
		std::vector<std::string> _uniformNames;

		bool compileShader(const std::string& filepath, GLuint& id);

		std::string _shaderName;

		GLuint _numAttribs;

		GLuint _programID;
		GLuint _vertexShaderID, _fragShaderID;

		bool _isBeingUsed;
	};

}