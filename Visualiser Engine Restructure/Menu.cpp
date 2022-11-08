#include "Menu.h"
#include "Errors.h"
#include "MyFuncs.h"

Sprite Menu::_background = Sprite();
GLSLProgram Menu::_noShadingProgram = GLSLProgram();
DropdownMenu* Menu::_drop = nullptr;

WindowInfo Menu::_windowInfo;

//PUBLIC
void Menu::init(WindowInfo windowInfo) {

	_windowInfo = windowInfo;

	_background.init(-1, -1, 2, 2, "Textures/awwhellnah.png");

	initShaders();
}


int Menu::processInput() {
	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) { //look up SDL_Event documentation to see other options for events (mouse movement, keyboard, etc..)
		case SDL_QUIT:
			return -1;
		case SDL_MOUSEBUTTONDOWN:
			std::vector<std::string> L;

			int x, y;
			SDL_GetMouseState(&x, &y);
			float normX, normY;
			MyFuncs::PixelCoordsToOpenGLcoords(x, y, normX, normY, _windowInfo);

			int out;
			DropdownMenu drop(L, normX, normY, 0.4, 0.7, _windowInfo, out);
			return out;
		}
	}
}

void Menu::draw() {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _windowInfo.screenWidth, _windowInfo.screenHeight);

	_noShadingProgram.use();
	_background.draw();
	_noShadingProgram.unuse();

	SDL_GL_SwapWindow(_windowInfo.window); //swaps the buffer in double buffer

}


//PRIVATE
void Menu::initShaders() {

	_noShadingProgram.compileShaders("Shaders/noshading.vert", "Shaders/noshading.frag");
	_noShadingProgram.addAttrib("vertexPosition");
	_noShadingProgram.addAttrib("vertexColour");
	_noShadingProgram.addAttrib("vertexUV");
	_noShadingProgram.linkShaders();
}
