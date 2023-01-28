#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>
#include <Vengine/Vengine.h>
#include <Vengine/MyImgui.h>

#include "SpriteManager.h"
#include "SignalProcessing.h"
#include "UI.h"

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
	Vengine::Viewport _viewport;

	int _sampleRate;
	GameState _gameState;
	UI _UI;

	Vengine::InputManager _inputManager;

	void initSystems();
	void initData();
	void processInput();
	void gameLoop();
	void drawVis();
	void drawUi();

	void endFrame();

	void initShaders();


	Vengine::Sprite _eq;
	Vengine::Sprite _background;
	SpriteManager _spriteManager;

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

	SignalProcessing _signalProc;

	Vengine::Audio _song;

	int _globalTimer;
};

