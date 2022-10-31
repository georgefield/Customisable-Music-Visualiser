#include "Menu.h"


//PUBLIC
void Menu::init() {

	_background.init(-1, -1, 2, 2);
}


void Menu::processInput() {

}

void Menu::draw(int screenWidth, int screenHeight) {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);

	_noShadingProgram.use();
	_background.draw();
	_noShadingProgram.unuse();
}


//PRIVATE
void Menu::initShaders() {

	_noShadingProgram.compileShaders("Shaders/noshading.vert", "Shaders/noshading.frag");
	_noShadingProgram.addAttrib("vertexPosition");
	_noShadingProgram.addAttrib("vertexColour");
	_noShadingProgram.addAttrib("vertexUV");
	_noShadingProgram.linkShaders();
}