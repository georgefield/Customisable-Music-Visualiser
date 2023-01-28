#pragma once
#include <glm/glm.hpp>

namespace Vengine {

	class Viewport {
	public:
		Viewport(int Width, int Height)
		{
			width = Width;
			height = Height;
		}

		int width;
		int height;

		void setDim(glm::vec2 wh) {
			width = wh.x;
			height = wh.y;
		}

		glm::vec2 getDim() {
			return glm::vec2(width, height);
		}

	};

}