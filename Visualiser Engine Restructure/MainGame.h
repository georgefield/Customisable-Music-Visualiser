#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>

#include "Sprite.h"
#include "GLSLProgram.h"
#include "GLtexture.h"
#include "ComputeShader.h"
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
	SDL_Window* _window;
	int _screenWidth;
	int _screenHeight;
	int _sampleRate;
	GameState _gameState;

	void initSystems();
	void processInput();
	void gameLoop();
	void drawGame();

	void initShaders();

	Sprite _screen;

	GLuint* _frameBufferIDs;
	GLuint* _frameBufferTextureIDs;
	const int _numFrameBuffers = 3;


	GLuint _ssboWavDataID;
	GLuint _ssboHarmonicDataID;
	GLuint _ssboAllocFFTmemID;


	std::vector<float> _wavData;
	std::vector<float> _harmonicData;

	float* _negArr;

	int _currSample;
	int _prevSample;
	float _sampleOffsetToSound;

	bool _parity;

	GLSLProgram _eqProgram;
	GLSLProgram _drawFrameBufferProgram;
	GLSLProgram _duplicateFrameProgram;
	GLSLProgram _shrinkScreenProgram;
	GLSLProgram _noShading;

	FFTW _fft;

	int _globalTimer;
};

