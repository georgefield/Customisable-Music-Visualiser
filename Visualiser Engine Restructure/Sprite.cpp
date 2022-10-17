#include "Sprite.h"
#include "Vertex.h"
#include "ResourceManager.h"

#include <cstddef>

Sprite::Sprite()
{
	_vboID = 0;
}


Sprite::~Sprite()
{
	if (_vboID != 0) {
		glDeleteBuffers(1, &_vboID); //remove all data associated with vboID from GPU
	}
}


void Sprite::init(float x, float y, float width, float height, std::string textureFilepath) {
	_x = x; //bottom left
	_y = y;
	_width = width;
	_height = height;

	if (textureFilepath != "") {
		_texture = ResourceManager::getTexture(textureFilepath);
	}
	else {
		_texture = GLtexture{ 0, 0, 0 };
	}

	if (_vboID == 0) { //get gl to generate vertex buffer id
		glGenBuffers(1, &_vboID);
	}

	//create quad (two triangles) (hard coded)
	Vertex vertexData[6];
	vertexData[0].setPosition(_x, _y + _height);//top left
	vertexData[0].setUV(0, 1); //uv is for texture mapping, range from 0 to 1, u is x axis, v is y axis, u,v = 0 is bottom left

	vertexData[1].setPosition(_x + _width, _y + _height);//top right
	vertexData[1].setUV(1, 1);

	vertexData[2].setPosition(_x,_y);//bottom left
	vertexData[2].setUV(0, 0);

	vertexData[3].setPosition(_x + _width, _y + _height);//top right
	vertexData[3].setUV(1, 1);

	vertexData[4].setPosition(_x, _y);//bottom left
	vertexData[4].setUV(0, 0);

	vertexData[5].setPosition(_x + _width, _y);//bottom right
	vertexData[5].setUV(1, 0);

	glBindBuffer(GL_ARRAY_BUFFER, _vboID); //bind _vboID to gl array buffer (can only have one array buffer active at one time)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), &vertexData, GL_STATIC_DRAW); //upload vertex data to GPU (bound to vboID)
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind _vboID
}

void Sprite::draw() {
	if (_texture.id != 0) {
		glBindTexture(GL_TEXTURE_2D, _texture.id);
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vboID);

	glEnableVertexAttribArray(0);

	//order matters, anytime there is vertex attribute you have to point openGL to the data in struct
	//position attribute pointer
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,position)); //stride is size of each vertex, offset is where in data position starts (0 as at start)
	//colour attribute pointer
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, colour)); //normalise as colour has been given in 0-255
	//uv attribute pointer
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glDrawArrays(GL_TRIANGLES, 0, 6); //triangles, start at 0th index, 6 vertices

	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}