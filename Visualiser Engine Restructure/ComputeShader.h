#pragma once
#include <GL/glew.h>
#include <string>

class ComputeShader
{
public:
	ComputeShader();
	~ComputeShader();

	void attachShader(const std::string& filepath);
	void run(int x, int y = 1, int z = 1);
	void use();
	GLuint getUniformLocation(const std::string& uniformName);
private:

	GLuint _outputTexID;

	GLuint _programID;
	GLuint _shaderID;
};

