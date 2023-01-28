#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <string>

#include <glm/glm.hpp>

namespace Vengine {

	class Window
	{
	public:
		Window();
		~Window();

		int create(std::string windowName, int screenWidth, int screenHeight, Uint32 flags);

		void nextFrame();
		void swapBuffer();

		//getters
		int getScreenWidth() const { return _screenWidth; }
		int getScreenHeight() const { return _screenHeight; }
		float getAspectRatio() const { return (float)_screenWidth / (float)_screenHeight; }

		glm::vec2 getScreenDim() const { return glm::vec2(_screenWidth, _screenHeight); }

		SDL_Window* getWindow() const { return _window; }
	private:

		void initImgui();

		SDL_Window* _window;
		SDL_GLContext _glContext;
		int _screenWidth, _screenHeight;
	};

}