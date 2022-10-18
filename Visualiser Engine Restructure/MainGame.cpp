#include "MainGame.h"
#include "Errors.h"
#include "IOManager.h"
#include "DrawFunctions.h"
#include "MyTiming.h"
#include "ResourceManager.h"

#include <iostream>
#include <string>
#include <Windows.h>

MainGame::MainGame() :
	_screenWidth(1024),
	_screenHeight(768),
	_window(nullptr),
	_gameState(GameState::PLAY),
	_currSample(0),
	_prevSample(-44100),
	_parity(true),
	_globalTimer(-1)
{
}

//RESTRUCTURE ENTIRE ENGINE, CLONE THIS PROJECT AS BACKUP

const std::string musicFilepath = "Music/Gorillaz - On Melancholy Hill.wav";

const int numFreq = 2048; //number of frequencies in the fourier transform (= half the number of samples included by nyquist)

void MainGame::run() {
	initSystems();

	//sprite init
	_quad.init(-1, -1, 2, 2);
	_quad2.init(-1, -1, 2, 2);
	_screen.init(-1, -1, 2, 2);
	_quadDupe.init(-1, -1, 2, 2); //little bit smaller
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

	//create SSBOs
	DrawFunctions::createSSBO(_ssboWavDataID, 0, &(_wavData[0]),  _wavData.size() * sizeof(float), GL_STATIC_COPY);

	_negArr = new float[numFreq];
	for (int i = 0; i < numFreq; i++) {
		_negArr[i] = -1.0f;
	}
	DrawFunctions::createSSBO(_ssboHarmonicDataID, 1, _negArr, numFreq * sizeof(float), GL_DYNAMIC_DRAW);


	_frameBufferIDs = new GLuint[_numFrameBuffers];
	_frameBufferTextureIDs = new GLuint[_numFrameBuffers];
	//create 2 draw buffers
	DrawFunctions::createDrawBuffers(_frameBufferIDs, _frameBufferTextureIDs, _screenWidth, _screenHeight, _numFrameBuffers);

	//debug for when doing ft on cpu
	_harmonicData.resize(numFreq);
}

void MainGame::initShaders() {
	_eqProgram.compileShaders("Shaders/eq.vert", "Shaders/eq.frag");
	//attributes must be added in order they are parsed in sprite draw()
	_eqProgram.addAttrib("vertexPosition");
	_eqProgram.addAttrib("vertexColour");
	_eqProgram.addAttrib("vertexUV");

	_eqProgram.linkShaders();

	//compute shader
	_eqCompute.attachShader("Compute/bruteFourier.compute");


	_drawFrameBufferProgram.compileShaders("Shaders/drawFrameBuffer.vert", "Shaders/drawFrameBuffer.frag");
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

	_noShading.linkShaders();
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

	MyTiming::startTimer(_globalTimer);

	printf("NOW1");
	PlaySound(musicFilepath.c_str() , NULL, SND_ASYNC);
	printf("NOW2");

	MyTiming::setNumSamplesForFPS(2);
	while (_gameState != GameState::EXIT) {

		processInput();
		drawGame();

		//frame done first then get fps
		MyTiming::frameDone();
		if (MyTiming::getFrameCount() % 100 == 0) { //every 100 frames print fps
			//printf("%f\n", MyTiming::getFPS());
		}
		//printf("%f\n",MyTiming::readTimer(_globalTimer));
	}
}

void MainGame::drawGame() {

	///compute current sample and run eq compute shader to find harmonic values
	float elapsed = MyTiming::readTimer(_globalTimer);
	_currSample = (int)(elapsed * _sampleRate);

	//ResourceManager::getFFT(_wavData, _currSample, _harmonicData);
	//DrawFunctions::updateSSBO(_ssboHarmonicDataID, 1, &(_harmonicData[0]), _harmonicData.size() * sizeof(float));

	_eqCompute.use();
	GLuint sampleLocation = _eqCompute.getUniformLocation("_currentSample");
	glUniform1i(sampleLocation, _currSample);
	_eqCompute.run(32); //4096/64 workgroup size
	glFinish();

	///draw fb2 and eq to fb1
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferIDs[1]);
	glViewport(0, 0, _screenWidth, _screenHeight);

	_duplicateFrameProgram.use();
	DrawFunctions::uploadTextureToShader(_duplicateFrameProgram, _frameBufferTextureIDs[2], "fb2"); //fb2

	_quadDupe.draw(); //draws to frame buffer 1
	_duplicateFrameProgram.unuse();

	///draw fb1 shrunk to fb2
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferIDs[2]);
	glViewport(0, 0, _screenWidth, _screenHeight);

	_shrinkScreenProgram.use();

	DrawFunctions::uploadTextureToShader(_shrinkScreenProgram, _frameBufferTextureIDs[1], "fb1");

	GLint timeLocationSS = _shrinkScreenProgram.getUniformLocation("time");
	glUniform1f(timeLocationSS, elapsed);

	_quad2.draw();

	_shrinkScreenProgram.unuse();

	///draw fb1 to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _screenWidth, _screenHeight);

	_drawFrameBufferProgram.use();

	DrawFunctions::uploadTextureToShader(_drawFrameBufferProgram, _frameBufferTextureIDs[1], "fb1");

	GLint timeLocationDFB = _drawFrameBufferProgram.getUniformLocation("time");
	glUniform1f(timeLocationDFB, elapsed);

	_screen.draw(); //draws to screen

	_drawFrameBufferProgram.unuse();

	SDL_GL_SwapWindow(_window); //swaps the buffer in double buffer

	//DrawFunctions::updateSSBO(_ssboDotProductDataID, 2, _zeroArr, numFreq * 2 * sizeof(float));

	//printf("%i,%i\n", _prevSample, _currSample);
	//system("PAUSE");
	/*
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _screenWidth, _screenHeight);

	_noShading.use();
	glActiveTexture(GL_TEXTURE1);
	GLint textureLocation = _noShading.getUniformLocation("tex");
	glUniform1i(textureLocation, 1); //0 as active texture is GL_TEXTURE0
	_quad2.draw();
	_noShading.unuse();

	SDL_GL_SwapWindow(_window); //swaps the buffer in double buffer*/
}