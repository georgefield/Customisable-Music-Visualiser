#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "BetterSprite.h"
#include <Vengine/Vengine.h>

class BetterSpriteBatch
{
public:
	BetterSpriteBatch();
	~BetterSpriteBatch();

	void init();

	void begin(Vengine::GlyphSortType sortType = Vengine::GlyphSortType::TEXTURE);
	void draw(BetterSprite* sprite);
	void end();
	void renderBatch();
private:
	void createRenderBatches();
	void createVertexArray();


	//comparison function for stable sort in "SortGlyphs()"
	static bool compareFrontToBack(BetterSprite* a, BetterSprite* b);
	static bool compareBackToFront(BetterSprite* a, BetterSprite* b);
	static bool compareTexture(BetterSprite* a, BetterSprite* b);
	//sort function
	void sortSprites();

	Vengine::GlyphSortType _sortType;

	GLuint _vbo;
	GLuint _vao;

	std::vector<BetterSprite*> _spritePtrs;

	std::vector<Vengine::RenderBatch> _renderBatches;
};