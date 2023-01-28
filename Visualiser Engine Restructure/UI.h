#pragma once
#include <Vengine/Vengine.h>
#include "SpriteManager.h"
#include <glm/glm.hpp>


class UI
{
public:
	UI();
	void init(Vengine::Window* window, SpriteManager* spriteManager, Vengine::InputManager* inputManager);

	void toolbar();
	void sidebar();
	void processInput();

	//getters
	bool getShowUi() const { return _showUi; }

	Vengine::Viewport getViewport();

private:
	Vengine::Window* _window;
	SpriteManager* _spriteManager;
	Vengine::InputManager* _inputManager;

	bool _showUi;
	//ImGui variables--
	int _toolbarSizePx, _sidebarSizePx;

	bool _save;
	bool _saveAs;
	bool _load;

	bool _fullscreen;
	//--
};

