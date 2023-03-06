#include "DataTextureCreator.h"
#include <GL/glew.h>
#include <iostream>


void DataTextureCreator::createTexture(int width, int height, float* data)
{
	if (numStoredTextures > 0) {
		Vengine::warning("Already created texture from data");
		return;
	}
	std::cout << "AYO";

	// Generate texture
	_texture.width = width;
	_texture.height = height;
	glGenTextures(1, &_texture.id);

	//bind texture
	glBindTexture(GL_TEXTURE_2D, _texture.id);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Upload data to texture object
	if (data == nullptr) { //upload zeros if nullptr
		data = new float[_texture.width * _texture.height];
		memset(data, 0.0f, _texture.width * _texture.height * sizeof(float));
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, _texture.width, _texture.height, 0, GL_RED, GL_FLOAT, data);

	//unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	numStoredTextures++;
}

void DataTextureCreator::updateTexture(int width, int height, int xOffset, int yOffset, float* data)
{
	glBindTexture(GL_TEXTURE_2D, _texture.id);

	glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, width, height, GL_RED, GL_FLOAT, data);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void DataTextureCreator::deleteTexture()
{
	if (numStoredTextures == 0) {
		Vengine::warning("No textures to delete");
		return;
	}
	//delete
	glDeleteTextures(1, &_texture.id);

	numStoredTextures--;
}
