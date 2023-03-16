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

	static void addSprite(Vengine::ModelType model);
	static void addSprite(CustomisableSprite::SpriteInfo spriteInfo);

	static void drawAll();

	static void deselectCurrent();
	static void select(int id);
	static void deleteSprite(int id);

	static void processInput(Vengine::InputManager* inputManager);

	static void updateDepthSortedSprites();

	//getters
	static std::vector<CustomisableSprite*> getDepthSortedSprites()  { return _depthSortedSpritePtrs; }

	static std::unordered_map<int, CustomisableSprite*>* getSpriteMap() { return &_userAddedSpritePtrs; }

private:
	static void drawNoBatching();

	static int _selectedSpriteId;

	static Vengine::Viewport* _viewport;
	static Vengine::Window* _window;

	//sprite containers
	static std::unordered_map<int, CustomisableSprite*> _userAddedSpritePtrs; //main container
	static std::vector<CustomisableSprite*> _depthSortedSpritePtrs; //work with this when rendering, updates to match main container when 'updateDepthSortSprites' called

	static GLuint _vao, _vbo;
};

