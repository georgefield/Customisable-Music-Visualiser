#pragma once
#include <GL/glew.h>

namespace Vengine {

	struct GLtexture {
		GLuint id;
		int width, height;

		bool valid() { return (width != 0) && (height != 0) && (id != 0); }
	};

}