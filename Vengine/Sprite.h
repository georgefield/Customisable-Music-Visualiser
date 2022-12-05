#pragma once
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

#include "GLtexture.h"
#include "Vertex.h"


namespace Vengine {

	class Sprite
	{
	public:
		Sprite();
		~Sprite();

		virtual void init(glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_STATIC_DRAW);

		virtual void draw();

	protected:
		float _depth;

		glm::vec2 _pos;
		glm::vec2 _dim;
		GLuint _vboID; //vertex buffer object id

		GLtexture _texture;

		Vertex _vertexData[6];

		void setRect(glm::vec2 pos, glm::vec2 dim);
	};

}