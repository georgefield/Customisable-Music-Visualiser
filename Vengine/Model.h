#pragma once
#include <glm/glm.hpp>

#include "Vertex.h"
#include "MyErrors.h"

namespace Vengine {

	//abstract model class
	class Model {
	public:
		~Model() { delete[] vertices; }
		glm::vec4 getBoundingBox() { return glm::vec4(pos, dim); }

		Vertex* vertices;
		int numVertices;
		glm::vec2 pos;
		glm::vec2 dim;

		//function to write for each
		virtual void init() = 0; //init to a default setting
		virtual void setBoundingBox(glm::vec2 Pos, glm::vec2 Dim) = 0; //set vertices to fill bounding rect
	};


	//basic quad model
	class Quad : public Model {
	public:
		void init() override {
			numVertices = 6;
			vertices = new Vertex[numVertices];

			setBoundingBox(glm::vec2(-0.5), glm::vec2(1));
		}

		void setBoundingBox(glm::vec2 Pos, glm::vec2 Dim) override;
	};

}