#include "MainProgram.h"

#include <iostream>
#include <string>
#include <Windows.h>

#include "FourierTransformManager.h"
#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"

const int screenWidth = 1024;
const int screenHeight = 768;

const int N = 4096; //number of frequencies in the fourier transform (= half the number of samples included by nyquist)

MainGame::MainGame() :
	_gameState(GameState::PLAY),
	_audio(),
	_UI(),
	_viewport(screenWidth, screenHeight)
{
}

//RESTRUCTURE ENTIRE ENGINE, CLONE THIS PROJECT AS BACKUP

const std::string musicFilepath = "Music/Gorillaz - On Melancholy Hill.wav";


void MainGame::run() {
	initSystems();
	initData();

	_eq.init(new Vengine::Quad(),glm::vec2(-1), glm::vec2(2));

	gameLoop();
}

void MainGame::initSystems() {

	//use Vengine to create window
	_window.create("visualiser", screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	SpriteManager::init(&_viewport, &_window);

	UniformSetting::init(&_signalProc);

	///---shader stuff

	initShaders();

	_spriteBatch.init();

	VisualiserManager::init();

	//load song & queue it
	_audio.loadWav(musicFilepath);
	_audio.queueLoadedWav();

	//set up signal processing unit
	_signalProc.init(_audio.getNormalisedWavData(), _audio.getSampleRate());
	FourierTransformManager::setMaster(_signalProc.getMaster());

	//create SSBO for waveform data (input entire file)
	Vengine::DrawFunctions::createSSBO(_ssboWavDataID, 0, _audio.getNormalisedWavData(), _audio.getWavLength(), GL_STATIC_COPY);

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

	_UI.init(&_window, &_inputManager, &_signalProc);

	//enable alpha belnding
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //blend entirely based on alpha
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void MainGame::initData(){

	//nothing for now
}

void MainGame::initShaders() {

	//shaders listed below are loaded on start up
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
		SpriteManager::processInput(&_inputManager);
	}
}

void MainGame::gameLoop() {

	Vengine::MyTiming::setNumSamplesForFPS(100);
	Vengine::MyTiming::setFPSlimit(2500);

	_signalProc._selfSimilarityMatrix.linkToMFCCs(&_signalProc._mfccs);
	//_signalProc._selfSimilarityMatrix.linkToDebug();

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
			if (_inputManager.isKeyPressed(SDLK_ESCAPE)) {
				if (_audio.isAudioPlaying()) {
					_audio.pause();
				}
				else {
					_audio.play();
				}
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

	static int correlationWindowSize = 1;
	if (_inputManager.isKeyPressed(SDLK_1)) {
		correlationWindowSize++;
		_signalProc._selfSimilarityMatrix.setCorrelationWindowSize(correlationWindowSize);
		std::cout << "+1\n";
	}

	_signalProc.beginCalculations(_audio.getCurrentSample());

	//note onset and tempo setup
	std::vector<int> FTs = FourierTransformManager::idArr();
	for (int i = 0; i < FTs.size(); i++) {
		FourierTransform* tmp = FourierTransformManager::getFourierTransform(FTs[i]);
		tmp->beginCalculation();
		//test->applyFunction(FourierTransform::SMOOTH);
		//test->applyFunction(FourierTransform::FREQUENCY_CONVOLVE);
		tmp->endCalculation();
	}


	//_signalProc._noteOnset.calculateNext();
	//_signalProc._tempoDetection.calculateNext();
	_signalProc._mfccs.calculateNext();
	_signalProc._selfSimilarityMatrix.calculateNext();

	_signalProc.endCalculations();

	//std::cout << _signalProc._selfSimilarityMatrix.getSelfSimilarityMatrixValue(0, 100) << std::endl;

	//Vengine::DrawFunctions::updateSSBO(_ssboHarmonicDataID, 1, test->getHistory()->newest(), test->getHistory()->numHarmonics() * sizeof(float));
	//_signalProc.updateSSBOwithVector(_signalProc._mfccs.getBandEnergy(), _ssboHarmonicDataID, 1);
	//_signalProc.updateSSBOwithHistory(_signalProc._tempoDetection.getConfidenceInTempoHistory(), _ssboHarmonicDataID, 1);
	if (_signalProc._tempoDetection.hasData()) {
		ImGui::Begin("tempo");
		ImGui::Text(std::to_string(_signalProc._tempoDetection.getTempoHistory()->newest()).c_str());
		ImGui::End();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_viewport.setDim(_UI.getViewport().getDim());
	glViewport(0,0,_viewport.width,_viewport.height);

	//batch if not showing ui
	SpriteManager::drawAll(!_UI.getShowUi());

	
	///draw eq to screen
	/*
	VisualiserShaderManager::getShader("Shaders/Preset/eq.vert", "Shaders/Preset/eq.frag")->getProgram()->use();

	GLint nLocation = Vengine::ResourceManager::getShaderProgram("Shaders/Preset/eq.vert", "Shaders/Preset/eq.frag")->getUniformLocation("n");
	glUniform1i(nLocation, N);

	_eq.draw(); //draws to screen

	Vengine::ResourceManager::getShaderProgram("Shaders/Preset/eq.vert", "Shaders/Preset/eq.frag")->unuse();
	*/
	VisualiserShaderManager::SSBOs::updateDynamicSSBOs();
	
}

void MainGame::drawUi(){

	_UI.errorMessages();
	_UI.toolbar();
	_UI.sidebar();
	_UI.processInput();
}
