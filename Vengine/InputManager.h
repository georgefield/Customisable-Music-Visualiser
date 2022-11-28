#pragma once
#include <unordered_map>
#include <glm/glm.hpp>

namespace Vengine {

	class InputManager
	{
	public:
		InputManager();

		void update();

		void pressKey(unsigned int keyID);
		void releaseKey(unsigned int keyID);

		void setMouseCoords(float x, float y);

		bool isKeyDown(unsigned int keyID); //true if key held down
		bool isKeyPressed(unsigned int keyID); //true if key just pressed
		bool isKeyReleased(unsigned int keyID);

		//getters
		glm::vec2 getMouseCoords() { return _mouseCoords; }
	private:
		bool wasKeyDown(unsigned int keyID);

		std::unordered_map<unsigned int, bool> _keyMap;
		std::unordered_map<unsigned int, bool> _prevKeyMap; //for checking iskeyPressed
		glm::vec2 _mouseCoords;
	};

}

