#pragma once
#include <map>
#include <string>
#include <vector>

#include <Vengine/Vengine.h>

#include "CustomisableSprite.h"

class SpriteManager
{
public:
	SpriteManager();
	~SpriteManager();

	void init(Vengine::Viewport* viewport, Vengine::Window* window);

	void addSprite(glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW);
	void deleteSprite(int id);

	void drawAll(bool batching);

	void processInput(Vengine::InputManager* inputManager);

	void updateDepthSortedSprites();

	//getters
	std::vector<CustomisableSprite*> getDepthSortedSprites()  { return _depthSortedSprites; }

private:
	void drawNoBatching();
	void drawWithBatching();

	int _selectedSpriteId;

	Vengine::Viewport* _viewport;
	Vengine::Window* _window;

	Vengine::SpriteBatch _spriteBatch;

	//sprite containers
	std::unordered_map<int, CustomisableSprite*> _userAddedSprites; //main container
	std::vector<CustomisableSprite*> _depthSortedSprites; //work with this when rendering, updates to match main container when 'updateDepthSortSprites' called

	GLuint _vao, _vbo;
};

