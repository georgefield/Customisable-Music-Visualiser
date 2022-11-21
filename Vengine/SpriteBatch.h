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


class Glyph
{
public:
	Glyph() {};
	Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, const GLuint& Texture, const float& Depth, const ColourRGBA8& colour) : 
		texture(Texture),
		depth(Depth) 
	{
		topLeft.colour = colour;
		topLeft.setPosition(destRect.x, destRect.y + destRect.w);
		topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

		bottomLeft.colour = colour;
		bottomLeft.setPosition(destRect.x, destRect.y);
		bottomLeft.setUV(uvRect.x, uvRect.y);

		bottomRight.colour = colour;
		bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
		bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

		topRight.colour = colour;
		topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
		topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);
	}

	Vertex topLeft;
	Vertex bottomLeft;
	Vertex topRight;
	Vertex bottomRight;

	GLuint texture;

	float depth;
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
	void draw(const glm::vec4& destRect, const glm::vec4& uvRect, const GLuint& texture, const float& depth, const ColourRGBA8& colour);
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

	std::vector<Glyph*> _glyphPtrs; // for sorting
	std::vector<Glyph> _glyphs; //actual glyphs

	std::vector<RenderBatch> _renderBatches;
};

}