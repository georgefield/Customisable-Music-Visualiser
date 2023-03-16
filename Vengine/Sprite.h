#pragma once
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

#include "GLtexture.h"
#include "Vertex.h"
#include "GLSLProgram.h"
#include "MyErrors.h"
#include "ResourceManager.h"
#include "Model.h"


namespace Vengine {


	class Sprite
	{
	public:
		Sprite();
		~Sprite();

		void init(ModelType model, glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_STATIC_DRAW);

		void draw();

		void updateBuffer();

		//getters
		float getDepth() {
			return _depth;
		}
		GLtexture* getTexture() {
			return &_texture;
		}
		Vertex* getVertices() {
			return _model->vertices;
		}
		int getNumVertices() {
			return _model->numVertices;
		}
		float _depth;

	private: 
		Model* _model;
	protected:

		GLuint _vboID; //vertex buffer object id

		GLtexture _texture;

		//model interaction helper functions
		//setters
		void setModelPos(glm::vec2 pos) {
			_model->setBoundingBox(pos, _model->dim);
			updateBuffer();
		}
		void setModelDim(glm::vec2 dim) {
			_model->setBoundingBox(_model->pos, dim);
			updateBuffer();
		}
		void setModelBoundingBox(glm::vec2 pos, glm::vec2 dim) {
			_model->setBoundingBox(pos, dim);
			updateBuffer();
		}
		void setModelColour(ColourRGBA8 colour) {
			_model->setColour(colour);
			updateBuffer();
		}
		//getters
		glm::vec4 getModelBoundingBox() {
			return _model->getBoundingBox();
		}
		glm::vec2 getModelPos() {
			return _model->pos;
		}
		glm::vec2 getModelDim() {
			return _model->dim;
		}
		ColourRGBA8 getModelColour() {
			return _model->colour;
		}

	};
}