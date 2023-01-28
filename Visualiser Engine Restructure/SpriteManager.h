#pragma once
#include <map>
#include <string>

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

	void draw();
	void drawWithBatching();

	void processInput(Vengine::InputManager* inputManager);
private:
	int _selectedSpriteId;

	Vengine::Viewport* _viewport;
	Vengine::Window* _window;

	Vengine::SpriteBatch _spriteBatch;
	std::map<int, CustomisableSprite*> _userAddedSprites;

	Vengine::GLSLProgram* _defaultShader;

	GLuint _vao, _vbo;
};

