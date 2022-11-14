#include "ComputeShader.h"
#include "GLSLProgram.h"
#include "Errors.h"

#include <fstream>
#include <vector>

using namespace Vengine;

ComputeShader::ComputeShader() :
	_programID(0),
	_shaderID(0)
{
}


ComputeShader::~ComputeShader()
{
}

void ComputeShader::attachShader(const std::string& filepath) {

	if (_shaderID != 0) {
		fatalError("Already attached shader to this compute shader class");
	}

	///---- load and compile shader
	_shaderID = glCreateShader(GL_COMPUTE_SHADER);

	std::ifstream shaderFile(filepath);
	if (shaderFile.fail()) {
		perror(filepath.c_str());
		fatalError("failed to open " + filepath);
	}
	std::string fileContents = "";
	std::string line;
	while (std::getline(shaderFile, line)) {
		fileContents += line + "\n";
	}
	shaderFile.close();

	const char* contentsPtr = fileContents.c_str();
	glShaderSource(_shaderID, 1, &contentsPtr, nullptr); //nullptr as only 1 string
	glCompileShader(_shaderID);
	//error checking for compilation
	GLint isCompiled = 0;
	glGetShaderiv(_shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(_shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(_shaderID, maxLength, &maxLength, &errorLog[0]);

		// Don't leak shaders
		glDeleteShader(_shaderID);

		// Provide the infolog in whatever manor you deem best.
		std::printf("%s\n", &(errorLog[0])); //interprets vector of chars as a string
		fatalError("compute shader " + filepath + " failed to compile");
	}

	///-----

	///----- link program 
	_programID = glCreateProgram();

	// Attach our shaders to our program
	glAttachShader(_programID, _shaderID);

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
		glDeleteShader(_shaderID);

		// Provide the infolog in whatever manor you deem best.
		std::printf("%s\n", &(errorLog[0])); //interprets vector of chars as a string
		fatalError("shaders failed to link");
	}

	// Always detach shaders after a successful link.
	glDetachShader(_programID, _shaderID);
}

void ComputeShader::run(int x, int y, int z) {

	glDispatchCompute(x, y, z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUseProgram(0);
}

void ComputeShader::use() {
	glUseProgram(_programID);
}

GLuint ComputeShader::getUniformLocation(const std::string& uniformName) {

	GLuint location = glGetUniformLocation(_programID, uniformName.c_str());
	if (location == GL_INVALID_INDEX) {
		fatalError("uniform " + uniformName + " not found in shader");
	}
	return location;
}
