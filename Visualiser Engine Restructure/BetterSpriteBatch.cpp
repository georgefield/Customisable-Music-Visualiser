#include "BetterSpriteBatch.h"

#include <algorithm>
#include "MyFuncs.h"

BetterSpriteBatch::BetterSpriteBatch() : _vbo(0), _vao(0)
{}

BetterSpriteBatch::~BetterSpriteBatch() {
	if (_vao != 0) {
		glDeleteVertexArrays(1, &_vao); //remove all data associated with vao from GPU
	}
	if (_vbo != 0) {
		glDeleteBuffers(1, &_vbo); //remove all data associated with vbo from GPU
	}
}


void BetterSpriteBatch::init() {

	createVertexArray();
}


void BetterSpriteBatch::begin() {

	_renderBatches.clear();
	_spritePtrs.clear();
}

void BetterSpriteBatch::end() {

	sortSprites();
	_cb = createRenderBatches();
}

void BetterSpriteBatch::draw(BetterSprite* sprite) { //adds glyph to vector of glyphs

	_spritePtrs.push_back(sprite);
}

void BetterSpriteBatch::renderBatch() {

	if (_vao == 0) {
		Vengine::fatalError("VAO not initialised so cannot render batch");
	}

	glBindVertexArray(_vao); //use already set up vao to handle attribs

	for (int i = 0; i < _cb; i++) {
		_renderBatches[i].first->use();
		MyFuncs::setUniformsForShader(_renderBatches[i].first);
		for (int j = 0; j < _renderBatches[i].second.size(); j++) {
			glBindTexture(GL_TEXTURE_2D, _renderBatches[i].second[j].texture);

			glDrawArrays(GL_TRIANGLES, _renderBatches[i].second[j].offset, _renderBatches[i].second[j].numVertices);
		}
		_renderBatches[i].first->unuse();
	}
	
	glBindVertexArray(0);
}

int BetterSpriteBatch::createRenderBatches() {

	if (_spritePtrs.empty()) {
		return 0;
	}

	std::vector<Vengine::Vertex> vertices;
	vertices.resize(_spritePtrs.size() * 6);

	int cv = 0, cs = 0, cb = 0; //current vertex, sprite, batch
	
	int offset = 0; //how many vertices added, jumps in 6s
	_renderBatches.resize(_spritePtrs.size());
	_renderBatches[cb].first = _spritePtrs[cs]->getShaderProgram();
	_renderBatches[cb].second.emplace_back(offset, 6, _spritePtrs[0]->getTexture().id);

	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	
	offset += 6;
	cs++;
	
	while (cs < _spritePtrs.size()) {
		if (_spritePtrs[cs]->getShaderProgram()->getID() != _spritePtrs[cs - 1]->getShaderProgram()->getID()) {
			cb++;
			_renderBatches[cb].first = _spritePtrs[cs]->getShaderProgram();
			_renderBatches[cb].second.emplace_back(offset, 6, _spritePtrs[cs]->getTexture().id);
		}
		else if (_spritePtrs[cs]->getTexture().id != _spritePtrs[cs - 1]->getTexture().id) {
			_renderBatches[cb].second.emplace_back(offset, 6, _spritePtrs[cs]->getTexture().id);
		}
		else {
			_renderBatches[cb].second.back().numVertices += 6;
		}

		vertices[cv] = _spritePtrs[cs]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cs]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cs]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cs]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cs]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cs]->getVertex(cv - offset); cv++;

		offset += 6;
		cs++;
	}
	
	///vvv send vertex data to GPU vvv
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	//orphan buffer
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vengine::Vertex), nullptr, GL_DYNAMIC_DRAW);
	//upload data of vertices array to vbo
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vengine::Vertex), vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return cb + 1; //num batches
}

//create vao and vbo
void BetterSpriteBatch::createVertexArray() { //vao is a way to tell opengl that all quads in vbo has same vertex attribs

	if (_vao != 0 || _vbo != 0) { Vengine::fatalError("Vertex array already created (SpriteBatch::init() probably called twice)"); }

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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vengine::Vertex), (void*)offsetof(Vengine::Vertex, position)); //stride is size of each vertex, offset is where in data position starts (0 as at start)
	//colour attribute pointer
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vengine::Vertex), (void*)offsetof(Vengine::Vertex, colour)); //normalise as colour has been given in 0-255
	//uv attribute pointer
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vengine::Vertex), (void*)offsetof(Vengine::Vertex, uv));


	//unbind
	glBindVertexArray(0);
}


//sorting stuff
bool BetterSpriteBatch::compareFrontToBack(BetterSprite* a, BetterSprite* b) {
	return (a->getDepth() < b->getDepth());
}
bool BetterSpriteBatch::compareBackToFront(BetterSprite* a, BetterSprite* b) {
	return (a->getDepth() > b->getDepth());
}
bool BetterSpriteBatch::compareTexture(BetterSprite* a, BetterSprite* b) {
	return (a->getTexture().id < b->getTexture().id); //sorts based on texture id (groups textures together) lower texture ids first (add first drawn first)
}
bool BetterSpriteBatch::compareShader(BetterSprite* a, BetterSprite* b) {
	return (a->getShaderProgram()->getID() < b->getShaderProgram()->getID()); //sorts based on texture id (groups textures together) lower texture ids first (add first drawn first)
}

void BetterSpriteBatch::sortSprites() {

	//by depth then split into shader then split into texture
	std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareTexture);
	std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareShader);
	std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareBackToFront);
}
