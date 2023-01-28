#include "Model.h"


using namespace Vengine;

//model functions

void Quad::setBoundingBox(glm::vec2 Pos, glm::vec2 Dim) {

	pos = Pos;
	dim = Dim;

	vertices[0].setPosition(pos.x, pos.y + dim.y);//top left
	vertices[0].setUV(0, 1); //uv is for texture mapping, range from 0 to 1, u is x axis, v is y axis, u,v = 0 is bottom left

	vertices[1].setPosition(pos.x + dim.x, pos.y + dim.y);//top right
	vertices[1].setUV(1, 1);

	vertices[2].setPosition(pos.x, pos.y);//bottom left
	vertices[2].setUV(0, 0);

	vertices[3].setPosition(pos.x + dim.x, pos.y + dim.y);//top right
	vertices[3].setUV(1, 1);

	vertices[4].setPosition(pos.x, pos.y);//bottom left
	vertices[4].setUV(0, 0);

	vertices[5].setPosition(pos.x + dim.x, pos.y);//bottom right
	vertices[5].setUV(1, 0);
}