#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "Vertex.h"
#include "GLtexture.h"

namespace Vengine {

enum class GlyphSortType {
	NONE, FRONT_TO_BACK, BACK_TO_FRONT, TEXTURE
};


struct Glyph
{
	float depth;

	Vertex topLeft;
	Vertex bottomLeft;
	Vertex topRight;
	Vertex bottomRight;

	GLuint texture;
};


class RenderBatch {
public: 
	RenderBatch(GLuint Offset, GLuint NumVertices, GLuint Texture) :
		offset(Offset), numVertices(NumVertices), texture(Texture)
	{
	}

	GLuint offset;
	GLuint numVertices;
	GLuint texture;
};


class SpriteBatch
{
public:
	SpriteBatch();

	void init();

	void begin(GlyphSortType sortType = GlyphSortType::TEXTURE);
	void end();
	void draw(const glm::vec4& destRect, const glm::vec4& uvRect, const GLuint& texture, const float& depth, const Colour& colour);
	void renderBatch();
private:
	void createRenderBatches();
	void createVertexArray();


	//comparison function for stable sort in "SortGlyphs()"
	static bool compareFrontToBack(Glyph* a, Glyph* b);
	static bool compareBackToFront(Glyph* a, Glyph* b);
	static bool compareTexture(Glyph* a, Glyph* b);
	//sort function
	void sortGlyphs();

	GlyphSortType _sortType;

	GLuint _vbo;
	GLuint _vao;

	std::vector<Glyph*> _glyphs;

	std::vector<RenderBatch> _renderBatches;
};

}