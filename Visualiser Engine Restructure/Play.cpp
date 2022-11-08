#include "Play.h"
#include "Errors.h"
#include "IOManager.h"
#include "DrawFunctions.h"
#include "MyTiming.h"
#include "ResourceManager.h"
#include "Audio.h"
#include "Menu.h"

#include <iostream>
#include <string>
#include <Windows.h>
const int N = 4096; //number of frequencies in the fourier transform (= half the number of samples included by nyquist)

WindowInfo Play::_windowInfo;

int Play::_sampleRate;
GLuint* Play::_frameBufferIDs;
GLuint* Play::_frameBufferTextureIDs;
const int Play::_numFrameBuffers = 3;

GLuint Play::_ssboWavDataID;
GLuint Play::_ssboHarmonicDataID;
GLuint Play::_ssboAllocFFTmemID;

std::vector<float> Play::_wavData;
std::vector<float> Play::_harmonicData;

float* Play::_negArr;

int Play::_currSample = 0;
int Play::_prevSample = -44100;
float Play::_sampleOffsetToSound = 0.0f;

GLSLProgram Play::_eqProgram;
GLSLProgram Play::_drawFrameBufferProgram;
GLSLProgram Play::_duplicateFrameProgram;
GLSLProgram Play::_shrinkScreenProgram;
GLSLProgram Play::_noShading;

FFTW Play::_fft(N);

int Play::_globalTimer = -1;


//RESTRUCTURE ENTIRE ENGINE, CLONE THIS PROJECT AS BACKUP

const std::string musicFilepath = "Music/King Geedorah - Next Levels.wav";


void Play::run() {
	initSystems();

	//sprite init
	_eq.init(-0.5, -0.5, 1, 1);
	_background.init(-1, -1, 2, 2, "Textures/awwhellnah.png");
	gameLoop();
}

void Play::initSystems() {

	SDL_Init(SDL_INIT_EVERYTHING);
	_windowInfo.window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _windowInfo.screenWidth, _windowInfo.screenHeight, SDL_WINDOW_OPENGL);

	if (_windowInfo.window == nullptr) {
		fatalError("SDL window could not be created");
	}

	//set up opengl context
	SDL_GLContext glContext = SDL_GL_CreateContext(_windowInfo.window);
	if (glContext == nullptr) {
		fatalError("SDL_GL context could not be created");
	}

	//optional, if hardware does not fully supports opengl glew will fix it
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		fatalError("Could not initialise glew");
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //tells SDL we want double buffer (avoids flickering/screen tearing)

	glClearColor(0.4f, 0.4f, 0.6f, 1.0f); //what colour the window clears to (rgba)

	initShaders();

	IOManager::loadWAV(musicFilepath, _wavData, _sampleRate);

	//create SSBO for waveform data (input entire file)
	DrawFunctions::createSSBO(_ssboWavDataID, 0, &(_wavData[0]), _wavData.size() * sizeof(float), GL_STATIC_COPY);

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
	DrawFunctions::createDrawBuffers(_frameBufferIDs, _frameBufferTextureIDs, _windowInfo.screenWidth, _windowInfo.screenHeight, _numFrameBuffers);


}

void Play::initShaders() {
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
	*/
	_noShading.compileShaders("Shaders/noshading.vert", "Shaders/noshading.frag");
	//attributes must be added in order they are parsed in sprite draw()
	_noShading.addAttrib("vertexPosition");
	_noShading.addAttrib("vertexColour");
	_noShading.addAttrib("vertexUV");

	_noShading.linkShaders();
}


void Play::processInput() {

	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) { //look up SDL_Event documentation to see other options for events (mouse movement, keyboard, etc..)
		case SDL_QUIT:
			_gameState = GameState::EXIT;
		}
	}
}

void Play::gameLoop() {


	Audio test;
	test.loadWav(musicFilepath);

	MyTiming::startTimer(_globalTimer);
	test.playSound();

	MyTiming::setNumSamplesForFPS(100);
	Menu::init(_windowInfo);

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

			if (Menu::processInput() == 2) {
				_gameState = GameState::PLAY;
			}
			Menu::draw();
		}
	}
}

void Play::draw() {

	///compute current sample and run eq compute shader to find harmonic values
	float elapsed = MyTiming::readTimer(_globalTimer);
	_currSample = max((int)(elapsed * _sampleRate) + _sampleOffsetToSound * _sampleRate, 0);

	_fft.getFFT(_wavData, _currSample, _harmonicData, 1000);

	DrawFunctions::updateSSBO(_ssboHarmonicDataID, 1, &(_harmonicData[0]), _harmonicData.size() * sizeof(float));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _windowInfo.screenWidth, _windowInfo.screenHeight);
	///draw background to screen
	_noShading.use();

	_background.draw();

	_noShading.unuse();
	///draw eq to screen
	_eqProgram.use();

	GLint nLocation = _eqProgram.getUniformLocation("n");
	glUniform1i(nLocation, N);

	_eq.draw(); //draws to screen

	_eqProgram.unuse();

	SDL_GL_SwapWindow(_windowInfo.window); //swaps the buffer in double buffer
}