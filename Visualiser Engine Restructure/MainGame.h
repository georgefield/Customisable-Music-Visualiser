#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>

#include "Sprite.h"
#include "GLSLProgram.h"
#include "GLtexture.h"
#include "ComputeShader.h"

enum class GameState {
	PLAY,
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

	Sprite _quad;
	Sprite _quad2;
	Sprite _quadDupe;
	Sprite _screen;

	GLuint* _frameBufferIDs;
	GLuint* _frameBufferTextureIDs;
	const int _numFrameBuffers = 3;


	GLuint _ssboWavDataID;
	GLuint _ssboHarmonicDataID;
	GLuint _ssboAllocFFTmemID;


	std::vector<float> _wavData;

	//debug vector for testing stuff
	std::vector<float> _harmonicData;

	float* _negArr;

	int _currSample;
	int _prevSample;
	bool _parity;

	GLSLProgram _eqProgram;
	ComputeShader _eqCompute;
	GLSLProgram _drawFrameBufferProgram;
	GLSLProgram _duplicateFrameProgram;
	GLSLProgram _shrinkScreenProgram;
	GLSLProgram _noShading;

	int _globalTimer;
};

