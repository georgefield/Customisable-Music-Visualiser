#include "Window.h"
#include "Errors.h"

using namespace Vengine;

Window::Window() {

}

Window::~Window() {

}

int Window::create(std::string windowName, int screenWidth, int screenHeight, Uint32 flags) {

	_screenWidth = screenWidth;
	_screenHeight = screenHeight;

	SDL_Init(SDL_INIT_EVERYTHING);

	//must be set before creating window
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //tells SDL we want double buffer (avoids flickering/screen tearing)

	_window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, flags);

	if (_window == nullptr) {
		fatalError("SDL window could not be created");
	}

	//set up opengl context
	SDL_GLContext glContext = SDL_GL_CreateContext(_window);
	if (glContext == nullptr) {
		fatalError("SDL_GL context could not be created");
	}

	//optional, if hardware does not fully supports opengl glew will fix it
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		fatalError("Could not initialise glew");
	}

	//print openGL version in console
	printf("***OpenGL version %s***\n", glGetString(GL_VERSION));

	//VSYNC!!!
	SDL_GL_SetSwapInterval(0); //1 on, 0 off

	//what colour the window clears to (rgba)
	glClearColor(0.4f, 0.4f, 0.6f, 1.0f);

	return 0; //could return error code for now just return 0
}


void Window::swapBuffer() {
	SDL_GL_SwapWindow(_window); //swaps the buffer in double buffer
}