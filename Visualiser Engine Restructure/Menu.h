#pragma once
#include "Sprite.h"
#include "GLSLProgram.h"

#include <GL/glew.h>

class Menu
{
public:
	static void init();
	static void processInput();
	static void draw(int screenWidth, int screenHeight);
private:
	static void initShaders();

	static Sprite _background;
	static GLSLProgram _noShadingProgram;
};

