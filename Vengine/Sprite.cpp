#include "Sprite.h"
#include "ResourceManager.h"
#include "MyErrors.h"

#include <cstddef>

using namespace Vengine;

Sprite::Sprite()
{
	_vboID = 0;
}


Sprite::~Sprite()
{
	if (_vboID != 0) {
		glDeleteBuffers(1, &_vboID); //remove all data associated with vboID from GPU
	}

	delete _model;
}

//helper function
Model* getModelObject(ModelType model) {
	if (model == Vengine::Mod_Quad)
		return new Vengine::Quad();
	if (model == Vengine::Mod_Triangle)
		return new Vengine::Triangle();
	if (model == Vengine::Mod_Ring)
		return new Vengine::Ring120side();
	if (model == Vengine::Mod_Circle)
		return new Vengine::Circle120side();
}
//

void Sprite::init(ModelType model, glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType) {

	_model = getModelObject(model);
	_model->init();
	_model->setBoundingBox(pos, dim);
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

	testForGlErrors("Error generating vbo");

	glBindBuffer(GL_ARRAY_BUFFER, _vboID); //bind _vboID to gl array buffer (can only have one array buffer active at one time)
	glBufferData(GL_ARRAY_BUFFER, _model->numVertices * sizeof(Vertex), _model->vertices, glDrawType); //upload vertex data to GPU (bound to vboID)
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind _vboID

	testForGlErrors("Error buffering data");
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

	glDrawArrays(_model->drawMode, 0, _model->numVertices); //triangles, start at 0th index, given num vertices

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0); //reset texture bind

	testForGlErrors("Error rendering sprite");
}


void Sprite::updateBuffer() {

	glBindBuffer(GL_ARRAY_BUFFER, _vboID); //bind _vboID to gl array buffer (can only have one array buffer active at one time)
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, _model->numVertices * sizeof(Vertex), _model->vertices);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind _vboID

	testForGlErrors("Error buffering data about model");
}


