#include "SpriteBatch.h"
#include "Errors.h"

#include <algorithm>

using namespace Vengine;

SpriteBatch::SpriteBatch() : _vbo(0), _vao(0)
{}


void SpriteBatch::init() {

	createVertexArray();
}


void SpriteBatch::begin(GlyphSortType sortType) {
	_sortType = sortType;
	_renderBatches.clear();
	for (int i = 0; i < _glyphs.size(); i++) {
		delete _glyphs[i];
	}
	_glyphs.clear();
}

void SpriteBatch::end() {
	sortGlyphs();
	createRenderBatches();
}

void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, const GLuint& texture, const float& depth, const ColourRGBA8& colour) { //adds glyph to vector of glyphs

	Glyph* newGlyph = new Glyph;

	newGlyph->texture = texture;
	newGlyph->depth = depth;

	newGlyph->topLeft.colour = colour;
	newGlyph->topLeft.setPosition(destRect.x, destRect.y + destRect.w);
	newGlyph->topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

	newGlyph->bottomLeft.colour = colour;
	newGlyph->bottomLeft.setPosition(destRect.x, destRect.y);
	newGlyph->bottomLeft.setUV(uvRect.x, uvRect.y);

	newGlyph->bottomRight.colour = colour;
	newGlyph->bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
	newGlyph->bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

	newGlyph->topRight.colour = colour;
	newGlyph->topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
	newGlyph->topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

	_glyphs.push_back(newGlyph);
}

void SpriteBatch::renderBatch() {
		
	if (_vao == 0) {
		fatalError("VAO not initialised so cannot render batch");
	}

	glBindVertexArray(_vao); //use already set up vao to handle attribs

	for (int i = 0; i < _renderBatches.size(); i++) {
		glBindTexture(GL_TEXTURE_2D, _renderBatches[i].texture);
		
		glDrawArrays(GL_TRIANGLES, _renderBatches[i].offset, _renderBatches[i].numVertices);
	}

	glBindVertexArray(0);
}


void SpriteBatch::createRenderBatches() {

	if (_glyphs.empty()) {
		fatalError("No glyphs stored in vector, nothing to render");
	}

	std::vector<Vertex> vertices;
	vertices.resize(_glyphs.size() * 6);

	//RenderBatch myBatch(0, 6, _glyphs[0]->texture); <--
	//_renderBatches.push_back(myBatch)               <-- inefficient (makes copy)

	int offset = 0;
	_renderBatches.emplace_back(offset, 6, _glyphs[0]->texture); //more efficient

	int cv = 0;
	vertices[cv++] = _glyphs[0]->topLeft;
	vertices[cv++] = _glyphs[0]->bottomLeft;
	vertices[cv++] = _glyphs[0]->bottomRight;
	vertices[cv++] = _glyphs[0]->bottomRight;
	vertices[cv++] = _glyphs[0]->topRight;
	vertices[cv++] = _glyphs[0]->topLeft;

	offset += 6;

	for (int cg = 1; cg < _glyphs.size(); cg++) {

		//save data of where vertices of quads with each texture is stored in vbo so can be called by draw arrays in render batch function
		if (_glyphs[cg]->texture != _glyphs[cg - 1]->texture) {
			_renderBatches.emplace_back(offset, 6, _glyphs[cg]->texture);
		}
		else {
			_renderBatches.back().numVertices += 6;
		}

		//add vertices to array
		vertices[cv++] = _glyphs[cg]->topLeft;
		vertices[cv++] = _glyphs[cg]->bottomLeft;
		vertices[cv++] = _glyphs[cg]->bottomRight;
		vertices[cv++] = _glyphs[cg]->bottomRight;
		vertices[cv++] = _glyphs[cg]->topRight;
		vertices[cv++] = _glyphs[cg]->topLeft;

		offset += 6;
	}

	///vvv send vertex data to GPU vvv
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	//orphan buffer
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
	//upload data of vertices array to vbo
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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


bool SpriteBatch::compareFrontToBack(Glyph* a, Glyph* b) {
	return (a->depth < b->depth);
}
bool SpriteBatch::compareBackToFront(Glyph* a, Glyph* b) {
	return (a->depth > b->depth);
}
bool SpriteBatch::compareTexture(Glyph* a, Glyph* b) {
	return (a->texture < b->texture); //sorts based on texture id (groups textures together) lower texture ids first (add first drawn first)
}

void SpriteBatch::sortGlyphs() {

	switch (_sortType) {
	case GlyphSortType::BACK_TO_FRONT:
		std::stable_sort(_glyphs.begin(), _glyphs.end(), compareBackToFront);
		break;
	case GlyphSortType::FRONT_TO_BACK:
		std::stable_sort(_glyphs.begin(), _glyphs.end(), compareFrontToBack);
		break;
	case GlyphSortType::TEXTURE:
		std::stable_sort(_glyphs.begin(), _glyphs.end(), compareTexture);
		break;
	}
}
