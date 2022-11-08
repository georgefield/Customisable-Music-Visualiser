#pragma once
#include "Sprite.h"
#include "GLSLProgram.h"
#include "DropdownMenu.h"
#include "WindowInfo.h"


#include <GL/glew.h>
#include <SDL/SDL.h>

class Menu
{
public:
	static void init(WindowInfo windowInfo);
	static int processInput();
	static void draw();
private:
	static void initShaders();

	static Sprite _background;
	static GLSLProgram _noShadingProgram;
	static DropdownMenu* _drop;

	static WindowInfo _windowInfo;
};

