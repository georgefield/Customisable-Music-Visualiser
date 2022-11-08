#pragma once
#include "WindowInfo.h"

class MyFuncs
{
public:
	static void PixelCoordsToOpenGLcoords(int x, int y, float& outX, float& outY, WindowInfo windowInfo);
};

