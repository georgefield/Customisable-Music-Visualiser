#pragma once
#include <Vengine/GLtexture.h>
#include <Vengine/MyErrors.h>

class DataTextureCreator {
public:
	DataTextureCreator() : numStoredTextures(0) {}
	void createTexture(int width, int height, float* data = nullptr);
	void updateTexture(int width, int height, int xOffset, int yOffset, float* data);
	void deleteTexture();

	bool isCreated() { return (numStoredTextures > 0); }

	Vengine::GLtexture getTexture() {
		if (!isCreated()) { Vengine::fatalError("Cannot get data texture before creating"); }
		return _texture;
	}

private:
	int numStoredTextures;
	Vengine::GLtexture _texture;
};