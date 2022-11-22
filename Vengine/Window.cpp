#include "Window.h"
#include "Errors.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

using namespace Vengine;

Window::Window() {

}

Window::~Window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
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
	_glContext = SDL_GL_CreateContext(_window);
	if (_glContext == nullptr) {
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//init imgui
	initImgui();

	return 0; //could return error code for now just return 0
}


void Window::nextFrame() {
	//clear last frame before rendering
	glClear(GL_COLOR_BUFFER_BIT);

	//tell imgui next frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void Vengine::Window::swapBuffer(){
	//swaps the buffer in double buffer
	SDL_GL_SwapWindow(_window);
}

void Vengine::Window::initImgui(){

	//init IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	
	ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
	ImGui_ImplOpenGL3_Init("#version 430");
}
