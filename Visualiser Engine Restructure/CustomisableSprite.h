#pragma once
#include "BetterSprite.h"
#include <string>
#include <glm/glm.hpp>

#include <Vengine/Vengine.h>

class CustomisableSprite : public BetterSprite
{
public:
	CustomisableSprite(const std::string& name, Vengine::Window* hostWindow);

	void draw() override;

	void processInput(Vengine::InputManager* inputManager);
private:

	Vengine::Window* _window;

	std::string _name;
	bool _isSelected;

	//selected/dragging vars--
	glm::vec2 _posOfMouseAtClick;
	glm::vec2 _posOfSpriteAtClick;
	int _timerID;
	bool _justShown;
	//--
};

