#pragma once
#include <GL/glew.h>
#include <string>

#include "GLtexture.h"

class Sprite
{
public:
	Sprite();
	~Sprite();

	void init(float x, float y, float width, float height, std::string textureFilepath = "");

	void draw();
private:
	float _x, _y, _width, _height;
	GLuint _vboID; //vertex buffer object id

	GLtexture _texture;
};

