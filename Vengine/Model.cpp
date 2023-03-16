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

void Vengine::Triangle::setBoundingBox(glm::vec2 Pos, glm::vec2 Dim)
{
	pos = Pos;
	dim = Dim;

	vertices[0].setPosition(pos.x, pos.y);
	vertices[0].setUV(0, 0);

	vertices[1].setPosition(pos.x + dim.x, pos.y);
	vertices[1].setUV(1, 0);

	vertices[2].setPosition(pos.x + (dim.x/2.0f), pos.y + dim.y);
	vertices[2].setUV(0.5 , 1);
}


void Vengine::Circle120side::setBoundingBox(glm::vec2 Pos, glm::vec2 Dim)
{
	pos = Pos;
	dim = Dim;

	const float PI = 3.1415926;

	glm::vec2 radius(dim.x / 2.0f, dim.y / 2.0f);
	glm::vec2 centre(pos.x + radius.x, pos.y + radius.y);

	float n = 120.0f;
	float angle = 2.0f * PI / n; // angle between two adjacent vertices

	int cv = 0;


	for (int i = 0; i < int(n); i++)
	{
		//edge vertices
		vertices[cv].setPosition(radius.x * cosf(i * angle) + centre.x, radius.y * sinf(i * angle) + centre.y);
		vertices[cv].setUV(float(i) / n, 1);
		cv++;

		//centre
		vertices[cv].setPosition(centre.x, centre.y);
		vertices[cv].setUV(float(i) / n, 0);
		cv++;
	}

	//back to start
	vertices[cv].setPosition(radius.x + centre.x, centre.y);
	vertices[cv].setUV(1, 1);
	cv++;

	assert(cv == numVertices);
}

void Vengine::Ring120side::setBoundingBox(glm::vec2 Pos, glm::vec2 Dim)
{
	pos = Pos;
	dim = Dim;

	const float PI = 3.1415926;

	glm::vec2 radius(dim.x / 2.0f, dim.y / 2.0f);
	glm::vec2 insideRadius(dim.x / 4.0f, dim.y / 4.0f);
	glm::vec2 centre(pos.x + radius.x, pos.y + radius.y);

	float n = 120.0f;
	float angle = 2.0f * PI / n; // angle between two adjacent vertices

	int cv = 0;

	for (int i = 0; i < int(n); i++)
	{
		//edge vertex
		vertices[cv].setPosition(radius.x * cosf(i * angle) + centre.x, radius.y * sinf(i * angle) + centre.y);
		vertices[cv].setUV(float(i) / n, 1);
		cv++;

		//centre vertex
		vertices[cv].setPosition(insideRadius.x * cosf(i * angle) + centre.x, insideRadius.y * sinf(i * angle) + centre.y);
		vertices[cv].setUV(float(i) / n, 0);
		cv++;
	}

	//back to start
	vertices[cv].setPosition(radius.x + centre.x, centre.y);
	vertices[cv].setUV(1, 1);
	cv++;

	vertices[cv].setPosition(insideRadius.x + centre.x, centre.y);
	vertices[cv].setUV(1, 0);
	cv++;

	assert(cv == numVertices);
}
