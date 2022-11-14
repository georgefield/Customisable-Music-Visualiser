#pragma once
#include <vector>
#include <string>
#include <SDL/SDL.h>

#include <Vengine/Sprite.h>
#include <Vengine/GLSLProgram.h>

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

	Vengine::GLSLProgram _singleColShading;
	Vengine::Sprite _backing;
	WindowInfo _windowInfo;
};

