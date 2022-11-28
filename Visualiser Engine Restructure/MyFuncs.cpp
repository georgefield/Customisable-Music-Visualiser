#include "MyFuncs.h"

Vengine::Window* MyFuncs::_window = nullptr;


void MyFuncs::PixelCoordsToOpenGLcoords(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH) {
	outXY = { inXY.x / screenWH.x, inXY.y / screenWH.y }; //get 0-1 space

	outXY.x = 2 * outXY.x - 1; //get opengl space
	outXY.y = 2 * -outXY.y + 1;
}

void MyFuncs::OpenGLcoordsToPixelCoords(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH){

	outXY.x = (inXY.x + 1.0f) / 2.0f; //get opengl space
	outXY.y = (inXY.y - 1.0f) / -2.0f;

	outXY = { outXY.x * screenWH.x, outXY.y * screenWH.y }; //get 0-1 space


}
