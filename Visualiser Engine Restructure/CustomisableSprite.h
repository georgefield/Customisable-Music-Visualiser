#pragma once
#include "BetterSprite.h"
#include <string>
#include <glm/glm.hpp>

#include <Vengine/Vengine.h>

enum SpriteComm {
	NOT_SELECTED,
	SELECTED,
	DELETE_SELF
};

class CustomisableSprite : public BetterSprite
{
public:
	CustomisableSprite(const std::string& name, Vengine::Window* hostWindow);

	void init(glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW) override;
	void draw() override;

	void processInput(Vengine::InputManager* inputManager);

	SpriteComm getSpriteState() { return _spriteState; }
private:
	bool posWithinSettings(glm::vec2 pos);

	void setOptionsWindowPosAndDim();

	//important information describing sprite--
	Vengine::Window* _window;
	std::string _name;	
	SpriteComm _spriteState;
	//--

	//imgui vars--
	glm::vec2 _optionsPosInPixels;
	glm::vec2 _optionsDimInPixels;

	bool _isOptionsEnlarged;
	int _minPixelsBetweenGUIandBottom;
	int _minPixelsBetweenGUIandRightSide;

	std::vector<std::string> _texFileNames;
	std::vector<std::string> _shaderFileNames;
	//--

	//selected/dragging vars--
	glm::vec2 _posOfMouseAtClick;
	glm::vec2 _posOfSpriteAtClick;
	int _timerID;
	bool _justCreated;
	//--

	//Attached Shader--
	Vengine::GLSLProgram* _shaderToUse;
	//--
};

