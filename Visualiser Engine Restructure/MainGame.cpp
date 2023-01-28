#include "MainGame.h"

#include <iostream>
#include <string>
#include <Windows.h>

#include "MyFuncs.h"

const int screenWidth = 1000;
const int screenHeight = 500;

const int N = 4096; //number of frequencies in the fourier transform (= half the number of samples included by nyquist)

MainGame::MainGame() :
	_gameState(GameState::PLAY),
	_currSample(0),
	_prevSample(-44100),
	_globalTimer(-1),
	_sampleOffsetToSound(-0.0f),
	_song(),
	_UI(),
	_viewport(screenWidth, screenHeight)
{
}

//RESTRUCTURE ENTIRE ENGINE, CLONE THIS PROJECT AS BACKUP

const std::string musicFilepath = "Music/King Geedorah - Next Levels.wav";


void MainGame::run() {
	initSystems();
	initData();

	_eq.init(new Vengine::Quad(),glm::vec2(-1), glm::vec2(2));
	_background.init(new Vengine::Quad(), glm::vec2(-1), glm::vec2(2), 0, "Textures/BlurryBackground.png");

	gameLoop();
}

void MainGame::initSystems() {

	//use Vengine to create window
	_window.create("visualiser", screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	_spriteManager.init(&_viewport, &_window);

	///---shader stuff

	initShaders();

	_spriteBatch.init();

	//load song
	_song.loadWav(musicFilepath, _sampleRate);

	//set up signal processing unit
	_signalProc.init(_song.getNormalisedWavData(), _sampleRate, SPflags::ALL);

	//create SSBO for waveform data (input entire file)
	Vengine::DrawFunctions::createSSBO(_ssboWavDataID, 0, _song.getNormalisedWavData(), _song.getWavLength(), GL_STATIC_COPY);

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

	_UI.init(&_window, &_spriteManager, &_inputManager);

	//enable alpha belnding
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //blend entirely based on alpha
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void MainGame::initData(){

	MyFuncs::setGlobalScreenDim(_window.getScreenWidth(), _window.getScreenHeight());
}

void MainGame::initShaders() {

	//shaders listed below are loaded on start up
	Vengine::ResourceManager::getShaderProgram("Shaders/eq");
	Vengine::ResourceManager::getShaderProgram("Shaders/wishyWashy");
}


void MainGame::processInput() {

	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		
		ImGui_ImplSDL2_ProcessEvent(&evnt); //pass events to imgui

		switch (evnt.type) { //look up SDL_Event documentation to see other options for events (mouse movement, keyboard, etc..)
		case SDL_QUIT:
			_gameState = GameState::EXIT;
			break;
		case SDL_KEYDOWN:
			_inputManager.pressKey(evnt.key.keysym.sym);
			break;
		case SDL_KEYUP:
			_inputManager.releaseKey(evnt.key.keysym.sym);
			break;
		case SDL_MOUSEBUTTONDOWN:
			_inputManager.pressKey(evnt.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			_inputManager.releaseKey(evnt.button.button);
			break;
		case SDL_MOUSEMOTION:
			_inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
		}
	}

	if (_UI.getShowUi()) {
		_spriteManager.processInput(&_inputManager);
	}
}

void MainGame::gameLoop() {

	Vengine::MyTiming::startTimer(_globalTimer);

	_song.playSound();

	Vengine::MyTiming::setNumSamplesForFPS(100);
	Vengine::MyTiming::setFPSlimit(2500);


	//main while loop
	while (_gameState != GameState::EXIT) {

		if (_gameState == GameState::PLAY) {
			_inputManager.update();
			//clears background and tells imgui next frame
			_window.nextFrame();

			processInput();

			if (_inputManager.isKeyPressed(SDL_BUTTON_LEFT)) {
				printf("%f fps\n", Vengine::MyTiming::getFPS());
				_signalProc.reset(); //fix all algorithms relying on continuity in case of build up of floating point errors
			}

			drawVis(); //visualiser first to draw ui on top of visualiser
			drawUi();

			endFrame();
		}
	}
}

void MainGame::endFrame() {
	//tell ImGui to render Ui (must do every frame)
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//display what has just been drawn to screen
	_window.swapBuffer();

	//to count frame timings
	Vengine::MyTiming::frameDone();
}


void MainGame::drawVis() {

	ImGui::ShowDemoWindow();

	///compute current sample and run eq compute shader to find harmonic values
	float elapsed = Vengine::MyTiming::readTimer(_globalTimer);
	_currSample = max((int)(elapsed * _sampleRate) + _sampleOffsetToSound * _sampleRate, 0);

	_signalProc.update(_currSample);
	//Vengine::DrawFunctions::updateSSBO(_ssboHarmonicDataID, 1, _signalProc.getFourierHarmonics(), _signalProc.getHowManyHarmonics() * sizeof(float));
	//Vengine::DrawFunctions::updateSSBO(_ssboHarmonicDataID, 1, _signalProc._convolvedFourierHarmonics.get(0), _signalProc.getHowManyHarmonics() * sizeof(float));
	_signalProc.updateSSBOwithHistory(&_signalProc._CONVderOfLogEnergy, _ssboHarmonicDataID, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_viewport.setDim(_UI.getViewport().getDim());
	glViewport(0,0,_viewport.width,_viewport.height);
	
	///sprite batch example
	Vengine::ResourceManager::getShaderProgram("Shaders/wishyWashy")->use();

	GLint timeLocation = Vengine::ResourceManager::getShaderProgram("Shaders/wishyWashy")->getUniformLocation("time");
	glUniform1f(timeLocation, elapsed);


	Vengine::ResourceManager::getShaderProgram("Shaders/wishyWashy")->unuse();

	
	//test better sprite
	if (_UI.getShowUi()) {
		_spriteManager.draw();
	}
	else {
		_spriteManager.drawWithBatching();
	}
	//--

	///draw eq to screen
	Vengine::ResourceManager::getShaderProgram("Shaders/eq")->use();

	GLint nLocation = Vengine::ResourceManager::getShaderProgram("Shaders/eq")->getUniformLocation("n");
	glUniform1i(nLocation, N);

	_eq.draw(); //draws to screen

	Vengine::ResourceManager::getShaderProgram("Shaders/eq")->unuse(); 
}

void MainGame::drawUi(){

	_UI.toolbar();
	_UI.sidebar();
	_UI.processInput();
}
