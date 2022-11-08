#include "MyFuncs.h"


void MyFuncs::PixelCoordsToOpenGLcoords(int x, int y, float& outX, float& outY, WindowInfo windowInfo) {
	outX = float(x) / float(windowInfo.screenWidth);
	outY = float(y) / float(windowInfo.screenHeight);
	outX = 2 * outX - 1;
	outY = 2 * -outY + 1;
}