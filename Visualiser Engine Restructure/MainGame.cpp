#include "MainGame.h"

#include <iostream>
#include <string>
#include <Windows.h>

const int N = 4096; //number of frequencies in the fourier transform (= half the number of samples included by nyquist)

MainGame::MainGame() :
	_gameState(GameState::PLAY),
	_currSample(0),
	_prevSample(-44100),
	_globalTimer(-1),
	_fft(N),
	_sampleOffsetToSound(-0.0f)
{
}

//RESTRUCTURE ENTIRE ENGINE, CLONE THIS PROJECT AS BACKUP

const std::string musicFilepath = "Music/King Geedorah - Next Levels.wav";


void MainGame::run() {
	initSystems();

	//sprite init
	_eq.init(-1, -1, 2, 2);
	_background.init(-1, -1, 2, 2, "Textures/awwhellnah.png");
	gameLoop();
}

void MainGame::initSystems() {

	//use Vengine to create window
	_window.create("visualiser", 1024, 768, SDL_WINDOW_OPENGL);


	///---shader stuff

	initShaders();

	_spriteBatch.init();

	Vengine::IOManager::loadWAV(musicFilepath, _wavData, _sampleRate);

	//create SSBO for waveform data (input entire file)
	Vengine::DrawFunctions::createSSBO(_ssboWavDataID, 0, &(_wavData[0]),  _wavData.size() * sizeof(float), GL_STATIC_COPY);

	//debug for when doing ft on cpu
	_harmonicData.resize((N / 2) + 1);

	//create harmonic amplitude array (fill with -1 for now, will be updated to current FFT every frame)
	_negArr = new float[_harmonicData.size()];
	//memset(_negArr, 0.5f, _harmonicData.size() * sizeof(float));
	for (int i = 0; i < _harmonicData.size(); i++) {
		_negArr[i] = 0.0f;
	}
	Vengine::DrawFunctions::createSSBO(_ssboHarmonicDataID, 1, _negArr, _harmonicData.size() * sizeof(float), GL_DYNAMIC_DRAW);


	_frameBufferIDs = new GLuint[_numFrameBuffers];
	_frameBufferTextureIDs = new GLuint[_numFrameBuffers];
	//create 2 draw buffers
	Vengine::DrawFunctions::createDrawBuffers(_frameBufferIDs, _frameBufferTextureIDs, _window.getScreenWidth(), _window.getScreenHeight(), _numFrameBuffers);


	//enable alpha belnding
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //blend entirely based on alpha
}

void MainGame::initShaders() {
	_eqProgram.compileShaders("Shaders/eq.vert", "Shaders/eq.frag");
	//attributes must be added in order they are parsed in sprite draw()
	_eqProgram.addAttrib("vertexPosition");
	_eqProgram.addAttrib("vertexColour");
	_eqProgram.addAttrib("vertexUV");

	_eqProgram.linkShaders();


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

	Vengine::Audio test;
	test.loadWav(musicFilepath);

	Vengine::MyTiming::startTimer(_globalTimer);

	test.playSound();

	Vengine::MyTiming::setNumSamplesForFPS(100);


	while (_gameState != GameState::EXIT) {

		if (_gameState == GameState::PLAY) {
			processInput();
			drawGame();

			//frame done first then get fps
			Vengine::MyTiming::frameDone();
			if (Vengine::MyTiming::getFrameCount() % 100 == 0) { //every 100 frames print fps
				printf("%f\n", Vengine::MyTiming::getFPS());
			}
		}
	}
}

void MainGame::drawGame() {

	///compute current sample and run eq compute shader to find harmonic values
	float elapsed = Vengine::MyTiming::readTimer(_globalTimer);
	_currSample = max((int)(elapsed * _sampleRate) + _sampleOffsetToSound * _sampleRate, 0);

	_fft.getFFT(_wavData, _currSample, _harmonicData, 1000);

	Vengine::DrawFunctions::updateSSBO(_ssboHarmonicDataID, 1, &(_harmonicData[0]), _harmonicData.size() * sizeof(float));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _window.getScreenWidth(), _window.getScreenHeight());
	
	///sprite batch example
	_noShading.use();

	_spriteBatch.begin(); //--- add quads to draw between begin and end

	//info of sprite to draw
	glm::vec4 pos(-1.0f, -1.0f, 2.0f, 2.0f);
	glm::vec4 uv(0, 0, 1.0f, 1.0f);
	Vengine::GLtexture texture = Vengine::ResourceManager::getTexture("Textures/awwhellnah.png");
	Vengine::Colour col = { 255,255,255,255 };
	//call draw command
	_spriteBatch.draw(pos, uv, texture.id, 0.0f, col);
	
	//randomised circles test -buggy interferes with eq vis somehow?????
	int N = 100;
	glm::vec4* posArr = new glm::vec4[N];
	Vengine::Colour* colArr = new Vengine::Colour[N];

	glm::vec4 uvC(0, 0, 1.0f, 1.0f);
	Vengine::GLtexture circTex = Vengine::ResourceManager::getTexture("Textures/100x100 red outlined circle.png");
	Vengine::Colour colC = { 255,255,255,255 };
	for (int i = 0; i < N; i++) {
		posArr[i] = glm::vec4(
			(float(rand()) / RAND_MAX) * 2.0f - 1.0f,
			(float(rand()) / RAND_MAX) * 2.0f - 1.0f,
			(float(rand()) / RAND_MAX) * 2.0f - 1.0f,
			(float(rand()) / RAND_MAX) * 2.0f - 1.0f);
		_spriteBatch.draw(posArr[i], uvC, circTex.id, 0.0f, colC);
	}
	
	_spriteBatch.end(); //--- sorts glyphs, then creates render batches

	_spriteBatch.renderBatch(); //draw all quads specified between begin and end to screen

	_noShading.unuse();


	///draw eq to screen
	_eqProgram.use();

	GLint nLocation = _eqProgram.getUniformLocation("n");
	glUniform1i(nLocation, N);

	_eq.draw(); //draws to screen

	_eqProgram.unuse();

	_window.swapBuffer();
}