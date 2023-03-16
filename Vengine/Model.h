#pragma once
#include <glm/glm.hpp>

#include "Vertex.h"
#include "MyErrors.h"

namespace Vengine {

	enum ModelType {
		Mod_Quad,
		Mod_Triangle,
		Mod_Circle,
		Mod_Ring
	};

	//abstract model class
	class Model {
	public:
		Model() { setColour({ 255,255,255,255 }); }
		~Model() { delete[] vertices; }
		glm::vec4 getBoundingBox() { return glm::vec4(pos, dim); }

		Vertex* vertices;
		int numVertices;
		glm::vec2 pos;
		glm::vec2 dim;
		ColourRGBA8 colour;

		ModelType modelType;
		GLenum drawMode;

		//function to write for each
		virtual void init() = 0; //init to a default setting
		virtual void setBoundingBox(glm::vec2 Pos, glm::vec2 Dim) = 0; //set vertices to fill bounding rect
		
		void setColour(ColourRGBA8 Colour) {
			colour = Colour;
			for (int i = 0; i < numVertices; i++) {
				vertices[i].setColour(colour.r, colour.g, colour.b, colour.a);
			}
		}
	};


	//basic quad model
	class Quad : public Model {
	public:
		void init() override {
			modelType = Mod_Quad;
			drawMode = GL_TRIANGLES;
			numVertices = 6;
			vertices = new Vertex[numVertices];

			setBoundingBox(glm::vec2(-0.5), glm::vec2(1));
		}

		void setBoundingBox(glm::vec2 Pos, glm::vec2 Dim) override;
	};

	class Triangle : public Model {
		void init() override {
			modelType = Mod_Triangle;
			drawMode = GL_TRIANGLES;
			numVertices = 3;
			vertices = new Vertex[numVertices];

			setBoundingBox(glm::vec2(-0.5), glm::vec2(1));
		}

		void setBoundingBox(glm::vec2 Pos, glm::vec2 Dim) override;
	};

	class Circle120side : public Model { //MAYBE FIX?
		void init() override {
			modelType = Mod_Circle;
			drawMode = GL_TRIANGLE_STRIP;
			numVertices = 241;
			vertices = new Vertex[numVertices];

			setBoundingBox(glm::vec2(-0.5), glm::vec2(1));
		}

		void setBoundingBox(glm::vec2 Pos, glm::vec2 Dim) override;
	};

	class Ring120side : public Model { //MAYBE FIX?
		void init() override {
			modelType = Mod_Ring;
			drawMode = GL_TRIANGLE_STRIP;
			numVertices = 242;
			vertices = new Vertex[numVertices];

			setBoundingBox(glm::vec2(-0.5), glm::vec2(1));
		}

		void setBoundingBox(glm::vec2 Pos, glm::vec2 Dim) override;
	};

}