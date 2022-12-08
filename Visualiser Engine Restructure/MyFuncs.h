#pragma once
#include <glm/glm.hpp>

#include <Vengine/Vengine.h>

#include <imgui.h>

class MyFuncs
{
public:
	static void setGlobalScreenDim(int w, int h) { _screenWH = { w, h }; };

	static bool posWithinRect(glm::vec2 pos, glm::vec4 rect); //does not matter whether in pix or openGL as long as both the same

	static void PixelCoordsToOpenGLcoords(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH = _screenWH);
	static void OpenGLcoordsToPixelCoords(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH = _screenWH);
	static void PixelSizeToOpenGLsize(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH = _screenWH);
	static void OpenGLsizeToPixelSize(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH = _screenWH);

	static void setUniformsForShader(Vengine::GLSLProgram* shaderProgram);
private:
	static glm::vec2 _screenWH;
};

