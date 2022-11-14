#include "DropdownMenu.h"

#include <Vengine/Errors.h>

#include "MyFuncs.h"


DropdownMenu::DropdownMenu(std::vector<std::string> options, float x, float y, float sizeX, float sizeY, WindowInfo windowInfo, int& codeOut) {

	_x = x; _y = y; _sizeX = sizeX; _sizeY = _sizeY;

	_windowInfo = windowInfo;

	_backing.init(x, y - sizeY, sizeX, sizeY);

	initShaders();

	codeOut = run();
}

DropdownMenu::~DropdownMenu() {
	printf("closing\n");
}

int DropdownMenu::processInput() {
	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) { //look up SDL_Event documentation to see other options for events (mouse movement, keyboard, etc..)
		case SDL_MOUSEBUTTONDOWN:
			int x, y;
			SDL_GetMouseState(&x, &y);
			float normX, normY;
			MyFuncs::PixelCoordsToOpenGLcoords(x, y, normX, normY, _windowInfo);

			if (normX >= _x && normY >= _y - _sizeY && normX <= _x + _sizeX && normY <= _y) {//mouse click in dropdown?
				printf("INSIDE\n");
				return 1;
			}
		case SDL_KEYDOWN:
			return 2;
		}
	}
	return 0;
}

int DropdownMenu::run() {
	int code = processInput();
	while (code != -1 && code != 2) {

		draw();
		code = processInput();
	}
	return code;
}


void DropdownMenu::draw() {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _windowInfo.screenWidth, _windowInfo.screenHeight);

	_singleColShading.use();

	GLint colLocation = _singleColShading.getUniformLocation("col");
	glUniform3f(colLocation, 1.0, 0.0, 0.0); //colour red for now

	_backing.draw();
	_singleColShading.unuse();

	SDL_GL_SwapWindow(_windowInfo.window); //swaps the buffer in double buffer
}


void DropdownMenu::initShaders() {

	_singleColShading.compileShaders("Shaders/singleColShading.vert", "Shaders/singleColShading.frag");
	_singleColShading.addAttrib("vertexPosition");
	_singleColShading.addAttrib("vertexColour");
	_singleColShading.addAttrib("vertexUV");
	_singleColShading.linkShaders();
}
