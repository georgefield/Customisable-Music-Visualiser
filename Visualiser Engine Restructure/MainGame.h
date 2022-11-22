#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>
#include <Vengine/Vengine.h>
#include <Vengine/MyImgui.h>

#include "FFTW.h"


enum class GameState {
	PLAY,
	MENU,
	EXIT
};

class MainGame
{
public:
	MainGame();

	void run();
private:
	Vengine::Window _window;

	int _sampleRate;
	GameState _gameState;

	Vengine::InputManager _inputManager;

	void initSystems();
	void initData();
	void processInput();
	void gameLoop();
	void drawVis();
	void drawUi();

	void initShaders();

	//ImGui variables
	float _yMult;
	bool _showUi;
	bool _showBackground;
	ImVec4 _clearColour;
	std::vector<std::string> _texFileNames;
	//--

	Vengine::Sprite _eq;

	Vengine::SpriteBatch _spriteBatch;

	GLuint* _frameBufferIDs;
	GLuint* _frameBufferTextureIDs;
	const int _numFrameBuffers = 3;


	GLuint _ssboWavDataID;
	GLuint _ssboHarmonicDataID;
	GLuint _ssboAllocFFTmemID;

	std::vector<float> _harmonicData;

	float* _negArr;

	int _currSample;
	int _prevSample;
	float _sampleOffsetToSound;


	Vengine::GLSLProgram _eqProgram;
	Vengine::GLSLProgram _noShading;
	Vengine::GLSLProgram _wishyWashyProgram;

	FFTW _fft;

	Vengine::Audio _song;

	int _globalTimer;
};

