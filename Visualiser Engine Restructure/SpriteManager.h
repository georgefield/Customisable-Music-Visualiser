#pragma once
#include <map>
#include <string>

#include <Vengine/Vengine.h>

#include "CustomisableSprite.h"
#include "BetterSpriteBatch.h"


class SpriteManager
{
public:
	SpriteManager();
	~SpriteManager();

	void init(Vengine::Window* hostWindow);

	void addSprite(glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW);
	void deleteSprite(int id);

	void draw();
	void drawWithBatching();

	void processInput(Vengine::InputManager* inputManager);
private:
	int _selectedSpriteId;

	Vengine::Window* _hostWindow;

	BetterSpriteBatch _spriteBatch;
	std::map<int, CustomisableSprite*> _userAddedSprites;

	GLuint _vao, _vbo;
};

