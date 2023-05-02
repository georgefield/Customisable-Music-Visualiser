#include "MainProgram.h"

#include <iostream>
#include <string>
#include <Windows.h>

#include "FourierTransformManager.h"
#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"
#include "VisPaths.h"


MainProgram::MainProgram() :
	_gameState(ProgramState::RUNNING),
	_UI(),
	_viewport(Vis::consts.defaultSW, Vis::consts.defaultSH)
{
}



void MainProgram::run() {
	init();
	gameLoop();
}

void MainProgram::init()
{
	initSystems();
	initManagers();
	initGenericUpdaters();
}

void MainProgram::initSystems() {

	//use Vengine to create window
	_window.create("visualiser", Vis::consts.defaultSW, Vis::consts.defaultSH, SDL_WINDOW_OPENGL);

	_frameBufferIDs = new GLuint[_numFrameBuffers];
	_frameBufferTextureIDs = new GLuint[_numFrameBuffers];
	//create 2 draw buffers
	Vengine::DrawFunctions::createDrawBuffers(_frameBufferIDs, _frameBufferTextureIDs, _window.getScreenWidth(), _window.getScreenHeight(), _numFrameBuffers);

	_UI.init(&_window, &_inputManager);

	//enable alpha belnding
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //blend entirely based on alpha
	glClearColor(0.1, 0.1, 0.1, 1.0); //slightly grey


	//timing and fps managing
	Vengine::MyTiming::setNumSamplesForFPS(100);
	Vengine::MyTiming::setFPSlimit(200);

	Vengine::MyTiming::createTimer(_timeSinceLoadTimerId);
	Vengine::MyTiming::startTimer(_timeSinceLoadTimerId);
}

void MainProgram::initManagers()
{
	VisualiserManager::init();
	VisualiserShaderManager::init();

	AudioManager::init();

	SignalProcessingManager::init();

	VisualiserManager::loadVisualiser(VisPaths::_startupVisualiserPath);

	SpriteManager::init(&_viewport, &_window);
}

void MainProgram::initGenericUpdaters()
{
	//time since load
	std::function<float()> timeSinceLoadUpdaterFunction = std::bind(&MainProgram::getTimeSinceLoad, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_timeSinceLoad", timeSinceLoadUpdaterFunction);

	//time in audio
	std::function<float()> timeInAudioUpdaterFunction = std::bind(&MainProgram::getTimeInAudio, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_timeInAudio", timeInAudioUpdaterFunction);

	//current sample
	std::function<int()> currentSampleUpdaterFunction = &AudioManager::getCurrentSample;
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_currentSample", currentSampleUpdaterFunction);

	//total samples
	std::function<int()> totalSamplesUpdaterFunction = &AudioManager::getTotalAudioDataLength;
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_totalSamples", totalSamplesUpdaterFunction);

	//sample rate
	std::function<int()> sampleRateUpdaterFunction = &AudioManager::getSampleRate;
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_sampleRate", sampleRateUpdaterFunction);
}

void MainProgram::processInput() {

	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		
		ImGui_ImplSDL2_ProcessEvent(&evnt); //pass events to imgui

		switch (evnt.type) { //look up SDL_Event documentation to see other options for events (mouse movement, keyboard, etc..)
		case SDL_QUIT:
			_gameState = ProgramState::EXIT;
			break;
		case SDL_KEYDOWN:
			if (!ImGui::GetIO().WantCaptureKeyboard) {
				_inputManager.pressKey(evnt.key.keysym.sym);
			}
			break;
		case SDL_KEYUP:
			if (!ImGui::GetIO().WantCaptureKeyboard) {
				_inputManager.releaseKey(evnt.key.keysym.sym);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!ImGui::GetIO().WantCaptureMouse) {
				_inputManager.pressKey(evnt.button.button);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (!ImGui::GetIO().WantCaptureMouse) {
				_inputManager.releaseKey(evnt.button.button);
			}
			break;
		case SDL_MOUSEMOTION:
			if (!ImGui::GetIO().WantCaptureMouse) {
				_inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
			}
		}
	}

	if (_UI.getShowUi() && !ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard) {
		SpriteManager::processInput(&_inputManager);
	}
}

void MainProgram::gameLoop() {

	//main while loop
	while (_gameState != ProgramState::EXIT) {

		if (_gameState == ProgramState::RUNNING) {
			_inputManager.update();
			//clears background and tells imgui next frame
			_window.nextFrame();

			processInput();

			if (_inputManager.isKeyPressed(SDL_BUTTON_LEFT)) {
				printf("%f fps\n", Vengine::MyTiming::getFPS());
			}
		

			drawVis(); //visualiser first to draw ui on top of visualiser
			drawUi();

			endFrame();
		}
	}
}

void MainProgram::endFrame() {

	//tell ImGui to render Ui (must do every frame)
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//display what has just been drawn to screen
	_window.swapBuffer();

	//needs to update information every frame
	AudioManager::updateCurrentAudioInfo();

	//to count frame timings
	Vengine::MyTiming::frameDone();
}


void MainProgram::drawVis() {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_viewport.setDim(_UI.getViewport().getDim());
	glViewport(0,0,_viewport.width,_viewport.height);

	//signal processing tmp
	SignalProcessingManager::calculate();

	//set vis values that will be sent to shader
	VisualiserShaderManager::updateUniformValuesToOutput();

	//batch if not showing ui
	SpriteManager::drawAll();

	VisualiserShaderManager::SSBOs::updateDynamicSSBOs();
	
}

void MainProgram::drawUi(){

	_UI.toolbar();
	_UI.sidebar();
	_UI.displayErrors();

	_UI.processInput();
}
