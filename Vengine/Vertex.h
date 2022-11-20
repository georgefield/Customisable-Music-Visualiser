#pragma once
#include <GL/glew.h>

namespace Vengine {

	struct Position {
		float x;
		float y;
	};

	struct ColourRGBA8 {
		ColourRGBA8() : r(0), b(0), g(0), a(255) {}
		ColourRGBA8(GLubyte R, GLubyte G, GLubyte B, GLubyte A) : r(R), g(G), b(B), a(A) {}

		GLubyte r = 0;
		GLubyte g = 0;
		GLubyte b = 0;
		GLubyte a = 0; //want number of bytes to be multiple of 4
	};

	struct UV {
		float u;
		float v;
	};

	struct Vertex {
		Position position;
		ColourRGBA8 colour;
		UV uv;

		void setPosition(float x, float y) {
			position.x = x; position.y = y;
		}
		void setColour(GLubyte r, GLubyte b, GLubyte g, GLubyte a) { //functions do not increase size of struct
			colour.r = r; colour.b = b; colour.g = g; colour.a = a;
		}
		void setUV(float u, float v) {
			uv.u = u; uv.v = v;
		}
	};

}