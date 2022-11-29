#include "BetterSpriteBatch.h"

#include <algorithm>

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


void BetterSpriteBatch::begin(Vengine::GlyphSortType sortType) {

	_sortType = sortType;
	_renderBatches.clear();
	_spritePtrs.clear();
}

void BetterSpriteBatch::end() {

	sortSprites();
	createRenderBatches();
}

void BetterSpriteBatch::draw(BetterSprite* sprite) { //adds glyph to vector of glyphs

	_spritePtrs.push_back(sprite);
}

void BetterSpriteBatch::renderBatch() {

	if (_vao == 0) {
		Vengine::fatalError("VAO not initialised so cannot render batch");
	}

	glBindVertexArray(_vao); //use already set up vao to handle attribs

	for (int i = 0; i < _renderBatches.size(); i++) {
		glBindTexture(GL_TEXTURE_2D, _renderBatches[i].texture);

		glDrawArrays(GL_TRIANGLES, _renderBatches[i].offset, _renderBatches[i].numVertices);
	}

	glBindVertexArray(0);
}


void BetterSpriteBatch::createRenderBatches() {

	if (_spritePtrs.empty()) {
		return;
	}

	std::vector<Vengine::Vertex> vertices;
	vertices.resize(_spritePtrs.size() * 6);

	//RenderBatch myBatch(0, 6, _glyphs[0]->texture); <--
	//_renderBatches.push_back(myBatch)               <-- inefficient (makes copy)

	int offset = 0;
	_renderBatches.emplace_back(offset, 6, _spritePtrs[0]->getTexture().id); //more efficient

	int cv = 0;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;
	vertices[cv] = _spritePtrs[0]->getVertex(cv); cv++;

	offset += 6;

	for (int cg = 1; cg < _spritePtrs.size(); cg++) {

		//save data of where vertices of quads with each texture is stored in vbo so can be called by draw arrays in render batch function
		if (_spritePtrs[cg]->getTexture().id != _spritePtrs[cg - 1]->getTexture().id) {
			_renderBatches.emplace_back(offset, 6, _spritePtrs[cg]->getTexture().id);
		}
		else {
			_renderBatches.back().numVertices += 6;
		}

		//add vertices to array
		vertices[cv] = _spritePtrs[cg]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cg]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cg]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cg]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cg]->getVertex(cv - offset); cv++;
		vertices[cv] = _spritePtrs[cg]->getVertex(cv - offset); cv++;

		offset += 6;
	}

	///vvv send vertex data to GPU vvv
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	//orphan buffer
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vengine::Vertex), nullptr, GL_DYNAMIC_DRAW);
	//upload data of vertices array to vbo
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vengine::Vertex), vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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

void BetterSpriteBatch::sortSprites() {

	switch (_sortType) {
	case Vengine::GlyphSortType::BACK_TO_FRONT:
		std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareBackToFront);
		break;
	case Vengine::GlyphSortType::FRONT_TO_BACK:
		std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareFrontToBack);
		break;
	case Vengine::GlyphSortType::TEXTURE:
		std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareTexture);
		break;
	case Vengine::GlyphSortType::BACK_TO_FRONT_BUT_GROUP_TEXTURE:
		std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareTexture);
		std::stable_sort(_spritePtrs.begin(), _spritePtrs.end(), compareBackToFront);
		break;
	}
}
