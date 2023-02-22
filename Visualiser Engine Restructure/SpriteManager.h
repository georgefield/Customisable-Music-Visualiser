#pragma once
#include <map>
#include <string>
#include <vector>

#include <Vengine/Vengine.h>

#include "CustomisableSprite.h"

class SpriteManager
{
public:
	static void reset();

	static void init(Vengine::Viewport* viewport, Vengine::Window* window);

	static void addSprite(glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW);
	static void deleteSprite(int id);

	static void drawAll(bool batching);

	static void processInput(Vengine::InputManager* inputManager);

	static void updateDepthSortedSprites();

	//getters
	static std::vector<CustomisableSprite*> getDepthSortedSprites()  { return _depthSortedSprites; }

private:
	static void drawNoBatching();
	static void drawWithBatching();

	static int _selectedSpriteId;

	static Vengine::Viewport* _viewport;
	static Vengine::Window* _window;

	static Vengine::SpriteBatch _spriteBatch;

	//sprite containers
	static std::unordered_map<int, CustomisableSprite*> _userAddedSprites; //main container
	static std::vector<CustomisableSprite*> _depthSortedSprites; //work with this when rendering, updates to match main container when 'updateDepthSortSprites' called

	static GLuint _vao, _vbo;
};

