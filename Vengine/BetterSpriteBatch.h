#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "Vertex.h"
#include "GLtexture.h"

namespace Vengine {

	enum class GlyphSortType {
		NONE, FRONT_TO_BACK, BACK_TO_FRONT, TEXTURE, BACK_TO_FRONT_BUT_GROUP_TEXTURE
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


	class BetterSpriteBatch
	{
	public:
		BetterSpriteBatch();
		~BetterSpriteBatch();

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

