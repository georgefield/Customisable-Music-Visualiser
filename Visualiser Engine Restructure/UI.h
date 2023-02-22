#pragma once
#include <Vengine/Vengine.h>
#include "SpriteManager.h"
#include <glm/glm.hpp>
#include <unordered_map>


class UI
{
public:
	UI();
	void init(Vengine::Window* window, Vengine::InputManager* inputManager);

	void toolbar();
	void sidebar();
	void errorMessages();
	void processInput();

	//getters
	bool getShowUi() const { return _showUi; }

	Vengine::Viewport getViewport();

private:

	Vengine::Window* _window;
	Vengine::InputManager* _inputManager;

	bool _showUi;
	//ImGui variables--
	int _toolbarSizePx, _sidebarSizePx;

	bool _save;
	bool _saveAs;
	bool _load;

	bool _fullscreen;
	//--

	int _errorMessageTimerId;

	void fileMenuAction();

	//helper gui functions
	bool textInputPrompt(const std::string& message, char* out);
	bool folderChooser(std::string startPath, std::string& out, bool loadFromOutsideStartPath);

	std::vector<std::string> _errorQueue;
};

