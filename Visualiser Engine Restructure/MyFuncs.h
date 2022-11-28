#pragma once
#include <glm/glm.hpp>
#include <Vengine/Window.h>

class MyFuncs
{
public:
	static void SetGlobalWindow(Vengine::Window& window) { _window = &window; }

	static void PixelCoordsToOpenGLcoords(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH);
	static void OpenGLcoordsToPixelCoords(glm::vec2 inXY, glm::vec2& outXY, glm::vec2 screenWH);
	
	static Vengine::Window* getWindow() { return _window; }
private:
	static Vengine::Window* _window;
};

