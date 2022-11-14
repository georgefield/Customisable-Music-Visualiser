#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <string>

namespace Vengine {

	class Window
	{
	public:
		Window();
		~Window();

		int create(std::string windowName, int screenWidth, int screenHeight, Uint32 flags);

		void swapBuffer();

		int getScreenWidth() { return _screenWidth; }
		int getScreenHeight() { return _screenHeight; }
	private:

		SDL_Window* _window;
		int _screenWidth, _screenHeight;
	};

}