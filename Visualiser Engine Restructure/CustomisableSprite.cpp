#include "CustomisableSprite.h"
#include <Vengine/MyImgui.h>

#include "MyFuncs.h"

CustomisableSprite::CustomisableSprite(const std::string& name, Vengine::Window* hostWindow) :
	_window(hostWindow),
	_name(name),
	_isSelected(true),
	_justShown(true)
{
}

void CustomisableSprite::draw(){

	//draw imgui
	if (_isSelected) {
		ImGui::Begin(_name.c_str());

		glm::vec2 windowPosInPix;
		MyFuncs::OpenGLcoordsToPixelCoords(glm::vec2(_pos.x, _pos.y), windowPosInPix, glm::vec2(1024, 768));
		ImGui::SetWindowPos(ImVec2(windowPosInPix.x, windowPosInPix.y));
		ImGui::SetWindowSize(ImVec2(100, 100));

		ImGui::End();
	}

	//draw quad
	BetterSprite::draw();
}


void CustomisableSprite::processInput(Vengine::InputManager* inputManager){

	glm::vec2 mousePos;
	MyFuncs::PixelCoordsToOpenGLcoords(inputManager->getMouseCoords(), mousePos, glm::vec2(_window->getScreenWidth(), _window->getScreenHeight()));

	//click down
	if (inputManager->isKeyPressed(SDL_BUTTON_LEFT)) {
		if (posWithinSprite(mousePos)) {
			Vengine::MyTiming::startTimer(_timerID);
			_posOfMouseAtClick = mousePos;
			_posOfSpriteAtClick = _pos;
		}
		else {
			_isSelected = false;
		}
	}

	//dragging while held down, selected
	if (_isSelected && inputManager->isKeyDown(SDL_BUTTON_LEFT) && posWithinSprite(mousePos)) {
		
		setRectPos(_posOfSpriteAtClick + (mousePos - _posOfMouseAtClick));
	}

	//click released
	if (!_justShown) {
		if (inputManager->isKeyReleased(SDL_BUTTON_LEFT) && posWithinSprite(mousePos)) {
			if (Vengine::MyTiming::readTimer(_timerID) < 0.4) {
				_isSelected = !_isSelected;
			}
		}
	}
	else {
		_justShown = false; //_justShown used so it doesnt immediantly deselect}
	}
}
