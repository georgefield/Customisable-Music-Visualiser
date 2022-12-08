#include "MainGame.h"

#include <iostream>
#include <string>
#include <Windows.h>

#include "MyFuncs.h"

const int N = 4096; //number of frequencies in the fourier transform (= half the number of samples included by nyquist)

MainGame::MainGame() :
	_gameState(GameState::PLAY),
	_currSample(0),
	_prevSample(-44100),
	_globalTimer(-1),
	_fft(N),
	_sampleOffsetToSound(-0.0f),
	_song(),
	_yMult(1.0f),
	_showUi(true),
	_showBackground(true),
	_clearColour(0, 0, 0, 1),
	_showDragableBox(false)
{
}

//RESTRUCTURE ENTIRE ENGINE, CLONE THIS PROJECT AS BACKUP

const std::string musicFilepath = "Music/Gorillaz - On Melancholy Hill.wav";


void MainGame::run() {
	initSystems();
	initData();
	//sprite init
	_eq.init(glm::vec2(-1), glm::vec2(2));

	gameLoop();
}

void MainGame::initSystems() {

	//use Vengine to create window
	_window.create("visualiser", 1024, 768, SDL_WINDOW_OPENGL);

	_spriteManager.init(&_window);

	///---shader stuff

	initShaders();

	_spriteBatch.init();

	//load song
	_song.loadWav(musicFilepath, _sampleRate);

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


	//enable alpha belnding
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //blend entirely based on alpha
}

void MainGame::initData(){

	Vengine::IOManager::getFilesInDir("Textures/", _texFileNames);
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

	if (_showUi) {
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
			}
			if (_inputManager.isKeyPressed(SDLK_TAB)) {
				_showUi = !_showUi;
			}

			drawVis(); //visualiser first to draw ui on top of visualiser
			if (_showUi) {
				drawUi();
			}
			//tell ImGui to render Ui (must do every frame)
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			//display what has just been drawn to screen
			_window.swapBuffer();

			//debug check fps
			Vengine::MyTiming::frameDone();
		}
	}
}

void MainGame::drawVis() {

	///compute current sample and run eq compute shader to find harmonic values
	float elapsed = Vengine::MyTiming::readTimer(_globalTimer);
	_currSample = max((int)(elapsed * _sampleRate) + _sampleOffsetToSound * _sampleRate, 0);

	_fft.getFFT(_song.getNormalisedWavData(), _currSample, _harmonicData, 500/_yMult);

	Vengine::DrawFunctions::updateSSBO(_ssboHarmonicDataID, 1, &(_harmonicData[0]), _harmonicData.size() * sizeof(float));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _window.getScreenWidth(), _window.getScreenHeight());
	
	///sprite batch example
	Vengine::ResourceManager::getShaderProgram("Shaders/wishyWashy")->use();

	GLint timeLocation = Vengine::ResourceManager::getShaderProgram("Shaders/wishyWashy")->getUniformLocation("time");
	glUniform1f(timeLocation, elapsed);

	_spriteBatch.begin(); //--- add quads to draw between begin and end

	if (_showBackground) {
		//info of sprite to draw
		glm::vec4 pos(-1.0f, -1.0f, 2.0f, 2.0f);
		glm::vec4 uv(0, 0, 1.0f, 1.0f);
		Vengine::GLtexture texture = Vengine::ResourceManager::getTexture("Textures/BlurryBackground.png");
		Vengine::ColourRGBA8 col = { 255,255,255,255 };
		//call draw command
		_spriteBatch.draw(pos, uv, texture.id, 0.0f, col);
	}
	
	
	//randomised circles test
	if (_inputManager.isKeyDown(SDLK_w)) {
		int N = 1000;
		glm::vec4* posArr = new glm::vec4[N];
		Vengine::ColourRGBA8* colArr = new Vengine::ColourRGBA8[N];

		glm::vec4 uvC(0, 0, 1.0f, 1.0f);
		Vengine::GLtexture circTex = Vengine::ResourceManager::getTexture("Textures/100x100 red outlined circle.png");
		Vengine::ColourRGBA8 colC = { 255,255,255,255 };
		for (int i = 0; i < N; i++) {
			posArr[i] = glm::vec4(
				(float(rand()) / RAND_MAX) * 2.0f - 1.0f,
				(float(rand()) / RAND_MAX) * 2.0f - 1.0f,
				(float(rand()) / RAND_MAX) * 2.0f - 1.0f,
				(float(rand()) / RAND_MAX) * 2.0f - 1.0f);
			_spriteBatch.draw(posArr[i], uvC, circTex.id, 0.0f, colC);
		}
	}
	//--

	_spriteBatch.end(); //--- sorts glyphs, then creates render batches

	_spriteBatch.renderBatch(); //draw all quads specified between begin and end to screen

	Vengine::ResourceManager::getShaderProgram("Shaders/wishyWashy")->unuse();

	
	//test better sprite
	if (_showUi) {
		ImGui::ShowDemoWindow();
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

	//create window
	ImGui::Begin("Tab to toggle UI");

	ImGui::Text("WIP menu");
	ImGui::SliderFloat("Y mult", &_yMult, 0.1f, 2.0f, "%f");
	ImGui::Checkbox("Show texture background", &_showBackground);

	//file chooser menu
	if (ImGui::Button("Add Sprite")) {
		_spriteManager.addSprite(glm::vec2(-0.5), glm::vec2(1));
	}
	//--

	//background colour picker
	ImGui::ColorPicker4("Clear Colour", (float*)&_clearColour, ImGuiColorEditFlags_NoAlpha);
	glClearColor(_clearColour.x, _clearColour.y, _clearColour.z, 1.0f);

	//end of window
	ImGui::End();
}
