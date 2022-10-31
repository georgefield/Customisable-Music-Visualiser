#include "MainGame.h"
#include "Errors.h"
#include "IOManager.h"
#include "DrawFunctions.h"
#include "MyTiming.h"
#include "ResourceManager.h"
#include "Audio.h"

#include <iostream>
#include <string>
#include <Windows.h>
const int N = 4096; //number of frequencies in the fourier transform (= half the number of samples included by nyquist)

MainGame::MainGame() :
	_screenWidth(1024),
	_screenHeight(768),
	_window(nullptr),
	_gameState(GameState::PLAY),
	_currSample(0),
	_prevSample(-44100),
	_parity(true),
	_globalTimer(-1),
	_fft(N),
	_sampleOffsetToSound(-0.3f)
{
}

//RESTRUCTURE ENTIRE ENGINE, CLONE THIS PROJECT AS BACKUP

const std::string musicFilepath = "Music/King Geedorah - Next Levels.wav";


void MainGame::run() {
	initSystems();

	//sprite init
	_screen.init(-1, -1, 2, 2);
	gameLoop();
}

void MainGame::initSystems() {

	SDL_Init(SDL_INIT_EVERYTHING);
	_window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_OPENGL);

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

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //tells SDL we want double buffer (avoids flickering/screen tearing)

	glClearColor(0.4f,0.4f,0.6f,1.0f); //what colour the window clears to (rgba)

	initShaders();

	IOManager::loadWAV(musicFilepath, _wavData, _sampleRate);

	//create SSBO for waveform data (input entire file)
	DrawFunctions::createSSBO(_ssboWavDataID, 0, &(_wavData[0]),  _wavData.size() * sizeof(float), GL_STATIC_COPY);

	//debug for when doing ft on cpu
	_harmonicData.resize((N / 2) + 1);

	//create harmonic amplitude array (fill with -1 for now, will be updated to current FFT every frame)
	_negArr = new float[_harmonicData.size()];
	//memset(_negArr, 0.5f, _harmonicData.size() * sizeof(float));
	for (int i = 0; i < _harmonicData.size(); i++) {
		_negArr[i] = 0.0f;
	}
	DrawFunctions::createSSBO(_ssboHarmonicDataID, 1, _negArr, _harmonicData.size() * sizeof(float), GL_DYNAMIC_DRAW);


	_frameBufferIDs = new GLuint[_numFrameBuffers];
	_frameBufferTextureIDs = new GLuint[_numFrameBuffers];
	//create 2 draw buffers
	DrawFunctions::createDrawBuffers(_frameBufferIDs, _frameBufferTextureIDs, _screenWidth, _screenHeight, _numFrameBuffers);


}

void MainGame::initShaders() {
	_eqProgram.compileShaders("Shaders/eq.vert", "Shaders/eq.frag");
	//attributes must be added in order they are parsed in sprite draw()
	_eqProgram.addAttrib("vertexPosition");
	_eqProgram.addAttrib("vertexColour");
	_eqProgram.addAttrib("vertexUV");

	_eqProgram.linkShaders();

	/*_drawFrameBufferProgram.compileShaders("Shaders/drawFrameBuffer.vert", "Shaders/drawFrameBuffer.frag");
	//attributes must be added in order they are parsed in sprite draw()
	_drawFrameBufferProgram.addAttrib("vertexPosition");
	_drawFrameBufferProgram.addAttrib("vertexColour");
	_drawFrameBufferProgram.addAttrib("vertexUV");

	_drawFrameBufferProgram.linkShaders();

	_duplicateFrameProgram.compileShaders("Shaders/duplicateFrame.vert", "Shaders/duplicateFrame.frag");
	//attributes must be added in order they are parsed in sprite draw()
	_duplicateFrameProgram.addAttrib("vertexPosition");
	_duplicateFrameProgram.addAttrib("vertexColour");
	_duplicateFrameProgram.addAttrib("vertexUV");

	_duplicateFrameProgram.linkShaders();

	_shrinkScreenProgram.compileShaders("Shaders/shrinkScreen.vert", "Shaders/shrinkScreen.frag");
	//attributes must be added in order they are parsed in sprite draw()
	_shrinkScreenProgram.addAttrib("vertexPosition");
	_shrinkScreenProgram.addAttrib("vertexColour");
	_shrinkScreenProgram.addAttrib("vertexUV");

	_shrinkScreenProgram.linkShaders();

	_noShading.compileShaders("Shaders/noshading.vert", "Shaders/noshading.frag");
	//attributes must be added in order they are parsed in sprite draw()
	_noShading.addAttrib("vertexPosition");
	_noShading.addAttrib("vertexColour");
	_noShading.addAttrib("vertexUV");

	_noShading.linkShaders();*/
}


void MainGame::processInput() {

	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) { //look up SDL_Event documentation to see other options for events (mouse movement, keyboard, etc..)
		case SDL_QUIT:
			_gameState = GameState::EXIT;
		}
	}
}

void MainGame::gameLoop() {


	Audio test;
	test.loadWav(musicFilepath);

	MyTiming::startTimer(_globalTimer);
	test.playSound();

	MyTiming::setNumSamplesForFPS(100);
	while (_gameState != GameState::EXIT) {

		if (_gameState == GameState::PLAY) {
			processInput();
			drawGame();

			//frame done first then get fps
			MyTiming::frameDone();
			if (MyTiming::getFrameCount() % 100 == 0) { //every 100 frames print fps
				printf("%f\n", MyTiming::getFPS());
			}
		}
		else if (_gameState == GameState::MENU) {

		}
	}
}

void MainGame::drawGame() {

	///compute current sample and run eq compute shader to find harmonic values
	float elapsed = MyTiming::readTimer(_globalTimer);
	_currSample = max((int)(elapsed * _sampleRate) + _sampleOffsetToSound*_sampleRate,0);

	_fft.getFFT(_wavData, _currSample, _harmonicData, 1000);

	DrawFunctions::updateSSBO(_ssboHarmonicDataID, 1, &(_harmonicData[0]), _harmonicData.size() * sizeof(float));

	///draw fb1 to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _screenWidth, _screenHeight);

	_eqProgram.use();

	GLint nLocation = _eqProgram.getUniformLocation("n");
	glUniform1i(nLocation, N);

	_screen.draw(); //draws to screen

	_eqProgram.unuse();

	SDL_GL_SwapWindow(_window); //swaps the buffer in double buffer
}