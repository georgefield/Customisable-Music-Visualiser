#include "SpriteBatch.h"
#include "MyErrors.h"

#include <algorithm>

using namespace Vengine;

SpriteBatch::SpriteBatch() : _vbo(0), _vao(0)
{}

SpriteBatch::~SpriteBatch() {
	if (_vao != 0) {
		glDeleteVertexArrays(1, &_vao); //remove all data associated with vao from GPU
	}
	if (_vbo != 0) {
		glDeleteBuffers(1, &_vbo); //remove all data associated with vbo from GPU
	}
}


void SpriteBatch::init() {

	createVertexArray();
}

void SpriteBatch::begin() {

	_programBatches.clear();
	_spritePtrs.clear();
}

void SpriteBatch::end() {
	
	sortSprites();
	createRenderBatches();
}

void SpriteBatch::draw(Sprite* sprite) { //adds glyph to vector of glyphs

	_spritePtrs.push_back(sprite);
	_numVerticesToDraw += sprite->getNumVertices();
}

void SpriteBatch::renderBatch(void (*uniformSetterFunction)(GLSLProgram*)) {

	if (_vao == 0) {
		Vengine::fatalError("VAO not initialised so cannot render batch");
	}

	glBindVertexArray(_vao); //use already set up vao to handle attribs

	for (int i = 0; i < _programBatches.size(); i++) {
		//use shader program for batch
		_programBatches[i].program->use();

		//set uniforms of shader
		uniformSetterFunction(_programBatches[i].program);

		for (int j = 0; j < _programBatches[i].textureBatches.size(); j++) {

			if (_programBatches[i].textureBatches[j].textureID != 0) { //set texture
				glBindTexture(GL_TEXTURE_2D, _programBatches[i].textureBatches[j].textureID);
			}

			//draw batch
			glDrawArrays(GL_TRIANGLES, _programBatches[i].textureBatches[j].batchOffset, _programBatches[i].textureBatches[j].numVertices);
		}

		_programBatches[i].program->unuse();
	}

	glBindVertexArray(0);
}

void addVerticesToContiguousArray(Vertex* array, Vertex* vertices, int numVertices, int& offset) {
	for (int i = 0; i < numVertices; i++) {
		array[offset + i] = vertices[i];
	}
	offset += numVertices;
}

void SpriteBatch::createRenderBatches() {

	if (_spritePtrs.empty()) {
		return;
	}

	Vertex* contiguousVertexArray = new Vertex[_numVerticesToDraw];
	int contiguousVertexArrayOffset = 0;

	int cs = 0; //current sprite
	int batchOffset = 0;


	_programBatches.emplace_back(); //new program batch
	_programBatches.back().program = _spritePtrs[cs]->getShaderProgram(); //set glsl program

	_programBatches.back().textureBatches.emplace_back(_spritePtrs[cs]->getTexture()->id, batchOffset); //new texture batch

	addVerticesToContiguousArray(contiguousVertexArray, _spritePtrs[cs]->getVertices(), _spritePtrs[cs]->getNumVertices(), contiguousVertexArrayOffset);
	_programBatches.back().textureBatches.back().numVertices += _spritePtrs[cs]->getNumVertices();

	cs++;

	while (cs < _spritePtrs.size()) {
		if (_spritePtrs[cs]->getShaderProgram()->getID() != _spritePtrs[cs - 1]->getShaderProgram()->getID()) { //new program batch

			batchOffset += _programBatches.back().textureBatches.back().numVertices;

			_programBatches.emplace_back();
			_programBatches.back().program = _spritePtrs[cs]->getShaderProgram();
			_programBatches.back().textureBatches.emplace_back(_spritePtrs[cs]->getTexture()->id, batchOffset); //new texture batch
		}
		else if (_spritePtrs[cs]->getTexture()->id != _spritePtrs[cs - 1]->getTexture()->id) {

			batchOffset += _programBatches.back().textureBatches.back().numVertices;

			_programBatches.back().textureBatches.emplace_back(_spritePtrs[cs]->getTexture()->id, batchOffset);
		}

		addVerticesToContiguousArray(contiguousVertexArray, _spritePtrs[cs]->getVertices(), _spritePtrs[cs]->getNumVertices(), contiguousVertexArrayOffset);
		_programBatches.back().textureBatches.back().numVertices += _spritePtrs[cs]->getNumVertices();

		cs++;
	}

	///vvv send vertex data to GPU vvv
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	//orphan buffer
	glBufferData(GL_ARRAY_BUFFER, _numVerticesToDraw * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
	//upload data of vertices array to vbo
	glBufferSubData(GL_ARRAY_BUFFER, 0, _numVerticesToDraw * sizeof(Vertex), contiguousVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] contiguousVertexArray;
}

void SpriteBatch::createVertexArray() { //vao is a way to tell opengl that all quads in vbo has same vertex attribs

	if (_vao != 0 || _vbo != 0) { fatalError("Vertex array already created (SpriteBatch::init() probably called twice)"); }

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	//---standard setup of vertex array
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	//order matters, anytime there is vertex attribute you have to point openGL to the data in struct
	//position attribute pointer
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)); //stride is size of each vertex, offset is where in data position starts (0 as at start)
	//colour attribute pointer
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, colour)); //normalise as colour has been given in 0-255
	//uv attribute pointer
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));


	//unbind
	glBindVertexArray(0);
}


//sorting stuff
bool SpriteBatch::compareFrontToBack(Sprite* a, Sprite* b) {
	return (a->getDepth() < b->getDepth());
}
bool SpriteBatch::compareBackToFront(Sprite* a, Sprite* b) {
	return (a->getDepth() > b->getDepth());
}
bool SpriteBatch::compareTexture(Sprite* a, Sprite* b) {
	return (a->getTexture()->id < b->getTexture()->id); //sorts based on texture id (groups textures together) lower texture ids first (add first drawn first)
}
bool SpriteBatch::compareShader(Sprite* a, Sprite* b) {
	return (a->getShaderProgram()->getID() < b->getShaderProgram()->getID()); //sorts based on texture id (groups textures together) lower texture ids first (add first drawn first)
}


void SpriteBatch::sortSprites() {

	//by depth then split into shader then split into texture
	std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareTexture);
	std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareShader);
	std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareBackToFront);
}

