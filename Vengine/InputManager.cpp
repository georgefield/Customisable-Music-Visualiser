#include "InputManager.h"


using namespace Vengine;

InputManager::InputManager() : _mouseCoords(0, 0), _isChangeThisFrame(false), _isKeyPressThisFrame(false) {

}

void InputManager::update(){
	_isChangeThisFrame = false;
	_isKeyPressThisFrame = false;
	//for (auto it = _keyMap.begin(); it != _keyMap.end()) {
	for (auto& it : _keyMap){ //iterate through keymap, does same as above
		_prevKeyMap[it.first] = it.second;
	}
}


void InputManager::pressKey(unsigned int keyID) {
	_keyMap[keyID] = true;
	_isChangeThisFrame = true;
	_isKeyPressThisFrame = true;
}

void InputManager::releaseKey(unsigned int keyID) {
	_keyMap[keyID] = false;
	_isChangeThisFrame = true;
}

void InputManager::setMouseCoords(float x, float y) {
	_mouseCoords.x = x;
	_mouseCoords.y = y;
	_isChangeThisFrame = true;
}

bool InputManager::isKeyDown(unsigned int keyID) {
	auto it = _keyMap.find(keyID);
	if (it != _keyMap.end()) {
		return it->second; //second value is the bool
	}
	else {
		return false;
	}
}

bool InputManager::isKeyPressed(unsigned int keyID){
	if (isKeyDown(keyID)) {
		if (!wasKeyDown(keyID)) {
			return true;
		}
	}
	return false;
}


bool InputManager::isKeyReleased(unsigned int keyID) {
	if (!isKeyDown(keyID)) {
		if (wasKeyDown(keyID)) {
			return true;
		}
	}
	return false;
}


bool InputManager::wasKeyDown(unsigned int keyID)
{
	auto it = _prevKeyMap.find(keyID);
	if (it != _prevKeyMap.end()) {
		return it->second; //second value is the bool
	}
	else {
		return false;
	}
}


