#include "GLSLProgram.h"
#include "Errors.h"

#include <fstream>
#include <iostream>
#include <vector>

using namespace Vengine;

GLSLProgram::GLSLProgram() : _programID(0), _vertexShaderID(0), _fragShaderID(0), _numAttribs(0)
{
}


GLSLProgram::~GLSLProgram()
{
}


void GLSLProgram::compileShaders(const std::string& vertShaderFilepath, const std::string& fragShaderFilepath) {

	_shaderName = fragShaderFilepath;

	_vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	if (_vertexShaderID == 0) {
		fatalError("failed to create vertex shader");
	}

	_fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	if (_fragShaderID == 0) {
		fatalError("failed to create fragment shader");
	}


	compileShader(vertShaderFilepath, _vertexShaderID);
	compileShader(fragShaderFilepath, _fragShaderID);

	// Get a program object. (needed for add attrib)
	_programID = glCreateProgram();
}


void GLSLProgram::linkShaders() {

	// Attach our shaders to our program
	glAttachShader(_programID, _vertexShaderID);
	glAttachShader(_programID, _fragShaderID);

	// Link our program
	glLinkProgram(_programID);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(_programID, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(_programID, maxLength, &maxLength, &errorLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(_programID);
		// Don't leak shaders either.
		glDeleteShader(_vertexShaderID);
		glDeleteShader(_fragShaderID);

		// Provide the infolog in whatever manor you deem best.
		std::printf("%s\n", &(errorLog[0])); //interprets vector of chars as a string
		fatalError("shaders failed to link");
	}

	// Always detach shaders after a successful link.
	glDetachShader(_programID, _vertexShaderID);
	glDetachShader(_programID, _fragShaderID);
}


void GLSLProgram::addAttrib(const std::string& attribName) {

	glBindAttribLocation(_programID, _numAttribs++, attribName.c_str());
}


GLuint GLSLProgram::getUniformLocation(const std::string& uniformName) {

	GLuint location = glGetUniformLocation(_programID, uniformName.c_str());
	if (location == GL_INVALID_INDEX) {
		fatalError("uniform " + uniformName + " not found in shader " + _shaderName);
	}
	return location;
}




void GLSLProgram::use() {
	glUseProgram(_programID);
	for (int i = 0; i < _numAttribs; i++) {
		glEnableVertexAttribArray(i);
	}
}
void GLSLProgram::unuse() {
	glUseProgram(0);
	for (int i = 0; i < _numAttribs; i++) {
		glDisableVertexAttribArray(i);
	}
}



void GLSLProgram::compileShader(const std::string& filepath, GLuint& id) {
	std::ifstream vertexFile(filepath);
	if (vertexFile.fail()) {
		perror(filepath.c_str());
		fatalError("failed to open " + filepath);
	}
	std::string fileContents = "";
	std::string line;
	while (std::getline(vertexFile, line)) {
		fileContents += line + "\n";
	}
	vertexFile.close();

	const char* contentsPtr = fileContents.c_str();
	glShaderSource(id, 1, &contentsPtr, nullptr); //nullptr as only 1 string
	glCompileShader(id);
	//error checking for compilation
	GLint isCompiled = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

		// Don't leak shaders
		glDeleteShader(id);

		// Provide the infolog in whatever manor you deem best.
		std::printf("%s\n", &(errorLog[0])); //interprets vector of chars as a string
		fatalError("shader " + filepath + " failed to compile");
	}
}