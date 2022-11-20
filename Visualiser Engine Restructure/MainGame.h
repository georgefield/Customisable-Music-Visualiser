#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>
#include <Vengine/Vengine.h>

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
	void processInput();
	void gameLoop();
	void drawGame();

	void initShaders();

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

