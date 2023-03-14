#include "Window.h"
#include "MyErrors.h"

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
	
	//initialise SDL
	printf("init SDL...\n");
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
	printf("init glew...\n");
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		fatalError("Could not initialise glew");
	}

	//general test for gl errors using glGetError func (in MyErrors)
	testForGlErrors("Problem setting up openGL");

	//print openGL version in console
	printf("***OpenGL version %s***\n", glGetString(GL_VERSION));

	//VSYNC!!!
	SDL_GL_SetSwapInterval(0); //1 on, 0 off

	//what colour the window clears to (rgba)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//init imgui
	printf("init imgui...\n");
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
	printf("	creating context...\n");
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	
	printf("	init SDL2...\n");
	ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
	printf("	init openGL3...\n");
	ImGui_ImplOpenGL3_Init();

	ImGui::CaptureMouseFromApp(true);
}
