#pragma once
#include <glm/glm.hpp>
#include <Vengine/Window.h>
#include <Vengine/Viewport.h>

class Tools
{
public:
	static bool posWithinRect(glm::vec2 pos, glm::vec4 rect)
	{
		if (pos.x >= rect.x && pos.x <= rect.x + rect.z
			&& pos.y >= rect.y && pos.y <= rect.y + rect.w) {
			return true;
		}
		return false;
	}




	//*** MATHS ***
	static float lerp(float c1, float c2, float t) {
		return ((1 - t) * c1) + (t * c2);
	}

	static float L1norm(float* v1, float* v2, int dim) {

		float ret = 0;
		for (int i = 0; i < dim; i++) {
			ret += fabsf(v1[i] - v2[i]);
		}
		return ret;
	}
	static float L2norm(float* v1, float* v2, int dim) {

		float ret = 0;
		for (int i = 0; i < dim; i++) {
			ret += sqrtf((v1[i] - v2[i]) * (v1[i] - v2[i]));
		}
		return ret;
	}
	static float L2normIncreasingDimensionsOnly(float* v1, float* v2, int dim) {

		float ret = 0;
		for (int i = 0; i < dim; i++) {
			if (v1[i] - v2[i] > 0) {
				ret += (v1[i] - v2[i]) * (v1[i] - v2[i]);
			}
		}
		return sqrt(ret);
	}
	static float HFCweightedL2normIncreasingDimensionsOnly(float* v1, float* v2, int dim) {

		float ret = 0;
		for (int i = 0; i < dim; i++) {
			if (v1[i] - v2[i] > 0) {
				ret += (i + 10) * (i + 10) * (v1[i] - v2[i]) * (v1[i] - v2[i]);
			}
		}
		return sqrt(ret);
	}




	//*** CONVERSIONS ***

		//---open gl to pixel---

	static glm::vec2 openGLposToPx(glm::vec2 openGLpos, Vengine::Window* window, Vengine::Viewport* viewport, bool ignoreViewport = false) {

		glm::vec2 pos = openGLpos;
		//0 -> 1 screen space BL (0,0)
		pos.x += 1.0f; pos.y += 1.0f;
		pos.x *= 0.5f; pos.y *= 0.5f;
		//flip y
		pos.y = 1.0f - pos.y;

		if (ignoreViewport) {
			pos.x *= window->getScreenWidth();
			pos.y *= window->getScreenHeight();
			return pos;
		}

		//scale
		pos.x *= viewport->width;
		pos.y *= viewport->height;
		//adjust as viewport from bottom left not top left
		pos.y += window->getScreenHeight() - viewport->height;

		return pos;
	}

	static glm::vec2 openGLdimToPx(glm::vec2 openGLdim, Vengine::Window* window, Vengine::Viewport* viewport, bool ignoreViewport = false) {

		glm::vec2 dim = openGLdim;

		//ignore viewport -> scale to window pix size as if viewport = window
		if (ignoreViewport) {
			dim.x *= window->getScreenWidth() * 0.5;
			dim.y *= window->getScreenHeight() * 0.5;
			return dim;
		}

		//scale
		dim.x *= viewport->width * 0.5;
		dim.y *= viewport->height * 0.5;
		//no other adjusting than scaling needed as it is a distance

		return dim;
	}

	static glm::vec4 openGLrectToPx(glm::vec4 openGLrect, Vengine::Window* window, Vengine::Viewport* viewport) {

		glm::vec2 pos = openGLposToPx({ openGLrect.x, openGLrect.y }, window, viewport);
		glm::vec2 dim = openGLdimToPx({ openGLrect.z, openGLrect.w }, window, viewport);

		return glm::vec4(pos, dim);
	}


	//---pixel to open gl---

	static glm::vec2 pxPosToOpenGL(glm::vec2 pxPos, Vengine::Window* window, Vengine::Viewport* viewport, bool ignoreViewport = false) {

		glm::vec2 pos = pxPos;

		if (ignoreViewport) {
			pos.x /= window->getScreenWidth();
			pos.y /= window->getScreenHeight();
		}
		else {
			pos.y -= window->getScreenHeight() - viewport->height;

			//get in 0->1 space
			pos.x /= viewport->width;
			pos.y /= viewport->height;
		}

		//flip y
		pos.y = 1.0f - pos.y;


		pos.x *= 2.0f; pos.y *= 2.0f;
		pos.x -= 1.0f; pos.y -= 1.0f;

		return pos;
	}

	static glm::vec2 pxDimToOpenGL(glm::vec2 pxDim, Vengine::Window* window, Vengine::Viewport* viewport, bool ignoreViewport = false) {

		glm::vec2 dim = pxDim;

		if (ignoreViewport) {
			dim.x /= window->getScreenWidth() * 0.5;
			dim.y /= window->getScreenHeight() * 0.5;
			return dim;
		}

		//get in 0->1 space
		dim.x /= viewport->width * 0.5;
		dim.y /= viewport->height * 0.5;

		return dim;
	}
};