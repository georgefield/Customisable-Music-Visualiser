#pragma once
#include <GL/glew.h>

struct Position {
	float x;
	float y;
};

struct Colour {
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
	Colour colour;
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