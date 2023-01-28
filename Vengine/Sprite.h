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

		void init(Model* model, glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_STATIC_DRAW);

		void draw();

		void attachShader(GLSLProgram* shaderProgram);

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

		//getters
		GLSLProgram* getShaderProgram() { 

			return _shaderProgram; 
		};

	private: 
		Model* _model;

	protected:

		float _depth;

		GLuint _vboID; //vertex buffer object id

		GLtexture _texture;

		GLSLProgram* _shaderProgram;

		//model interaction helper functions
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
		glm::vec4 getModelBoundingBox() {
			return _model->getBoundingBox();
		}
		glm::vec2 getModelPos() {
			return _model->pos;
		}
		glm::vec2 getModelDim() {
			return _model->dim;
		}

	};
}