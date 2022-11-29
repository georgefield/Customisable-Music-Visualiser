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

	void addSprite(const std::string& name);
	void deleteSprite(const std::string& name);
	
	void initSprite(const std::string& name, glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW);

	void draw();
	void drawWithBatching();

	void processInput(Vengine::InputManager* inputManager);

	CustomisableSprite* getSprite(std::string name);
private:
	Vengine::Window* _hostWindow;

	BetterSpriteBatch _spriteBatch;
	std::map<std::string, CustomisableSprite*> _userAddedSprites;

	GLuint _vao, _vbo;
};

