#include "Sprite.h"
#include "ResourceManager.h"
#include "MyErrors.h"

#include <cstddef>

using namespace Vengine;

Sprite::Sprite() :
	_pos(0,0),
	_dim(0,0)
{
	_vboID = 0;
}


Sprite::~Sprite()
{
	if (_vboID != 0) {
		glDeleteBuffers(1, &_vboID); //remove all data associated with vboID from GPU
	}
}


void Sprite::init(glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType) {

	_depth = depth;

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
	setRect(pos, dim);

	glBindBuffer(GL_ARRAY_BUFFER, _vboID); //bind _vboID to gl array buffer (can only have one array buffer active at one time)
	glBufferData(GL_ARRAY_BUFFER, sizeof(_vertexData), &_vertexData, glDrawType); //upload vertex data to GPU (bound to vboID)
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind _vboID
}

void Sprite::draw() {
	if (_texture.id != 0) {
		glBindTexture(GL_TEXTURE_2D, _texture.id);
	}
	else {
		//warning("Binding texture_2d to id '0' as texture not set for the sprite");
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vboID);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	//order matters, anytime there is vertex attribute you have to point openGL to the data in struct
	//position attribute pointer
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,position)); //stride is size of each vertex, offset is where in data position starts (0 as at start)
	//colour attribute pointer
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, colour)); //normalise as colour has been given in 0-255
	//uv attribute pointer
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glDrawArrays(GL_TRIANGLES, 0, 6); //triangles, start at 0th index, 6 vertices

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0); //reset texture bind
}

void Vengine::Sprite::setRect(glm::vec2 pos, glm::vec2 dim)
{
	_pos = pos;
	_dim = dim;

	_vertexData[0].setPosition(pos.x, pos.y + dim.y);//top left
	_vertexData[0].setUV(0, 1); //uv is for texture mapping, range from 0 to 1, u is x axis, v is y axis, u,v = 0 is bottom left

	_vertexData[1].setPosition(pos.x + dim.x, pos.y + dim.y);//top right
	_vertexData[1].setUV(1, 1);

	_vertexData[2].setPosition(pos.x, pos.y);//bottom left
	_vertexData[2].setUV(0, 0);

	_vertexData[3].setPosition(pos.x + dim.x, pos.y + dim.y);//top right
	_vertexData[3].setUV(1, 1);

	_vertexData[4].setPosition(pos.x, pos.y);//bottom left
	_vertexData[4].setUV(0, 0);

	_vertexData[5].setPosition(pos.x + dim.x, pos.y);//bottom right
	_vertexData[5].setUV(1, 0);
}
