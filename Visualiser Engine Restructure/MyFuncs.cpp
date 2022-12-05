#include "MyFuncs.h"

#include <Vengine/MyErrors.h>

glm::vec2 MyFuncs::_screenWH = { 0, 0 };


bool MyFuncs::posWithinRect(glm::vec2 pos, glm::vec4 rect)
{
	if (pos.x >= rect.x && pos.x <= rect.x + rect.z
		&& pos.y >= rect.y && pos.y <= rect.y + rect.w) {
		return true;
	}
	return false;
}


void MyFuncs::PixelCoordsToOpenGLcoords(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH) {
	if (screenWH.x <= 0 || screenWH.y <= 0) { Vengine::fatalError("invalid screenWH: probably forgot to use setGlobalScreenDim()"); }

	outXY = { inXY.x / screenWH.x, inXY.y / screenWH.y }; //get 0-1 space

	outXY.x = 2 * outXY.x - 1; //get opengl space
	outXY.y = 2 * -outXY.y + 1;
}

void MyFuncs::OpenGLcoordsToPixelCoords(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH){
	if (screenWH.x <= 0 || screenWH.y <= 0) { Vengine::fatalError("invalid screenWH: probably forgot to use setGlobalScreenDim()"); }

	outXY.x = (inXY.x + 1.0f) / 2.0f; //get opengl space
	outXY.y = (inXY.y - 1.0f) / -2.0f;

	outXY = { outXY.x * screenWH.x, outXY.y * screenWH.y }; //get 0-1 space
}

void MyFuncs::PixelSizeToOpenGLsize(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH){
	if (screenWH.x <= 0 || screenWH.y <= 0) { Vengine::fatalError("invalid screenWH: probably forgot to use setGlobalScreenDim()"); }

	outXY = { inXY.x * 2.0f/ screenWH.x, inXY.y * 2.0f / screenWH.y };
}

void MyFuncs::OpenGLsizeToPixelSize(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH){
	if (screenWH.x <= 0 || screenWH.y <= 0) { Vengine::fatalError("invalid screenWH: probably forgot to use setGlobalScreenDim()"); }

	outXY = { inXY.x * 0.5f * screenWH.x, inXY.y * 0.5f * screenWH.y };
}