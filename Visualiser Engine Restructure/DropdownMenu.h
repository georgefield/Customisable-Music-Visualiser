#pragma once
#include <vector>
#include <string>
#include <SDL/SDL.h>

#include "Sprite.h"
#include "GLSLProgram.h"
#include "WindowInfo.h"

class DropdownMenu
{
public:
	DropdownMenu(std::vector<std::string> options, float x, float y, float sizeX, float sizeY, WindowInfo windowInfo, int& codeOut);
	~DropdownMenu();

private:
	void initShaders();
	int run();
	void draw();
	int processInput();

	float _x, _y, _sizeX, _sizeY;

	GLSLProgram _singleColShading;
	Sprite _backing;
	WindowInfo _windowInfo;
};

