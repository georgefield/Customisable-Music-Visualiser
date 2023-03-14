#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>
#include <Vengine/Vengine.h>
#include <Vengine/MyImgui.h>

#include "SpriteManager.h"
#include "SignalProcessingManager.h"
#include "UI.h"

enum class ProgramState {
	RUNNING,
	MENU,
	EXIT
};

class MainProgram
{
public:
	MainProgram();

	void run();
private:
	Vengine::Window _window;
	Vengine::Viewport _viewport;

	int _sampleRate;
	ProgramState _gameState;
	UI _UI;

	Vengine::InputManager _inputManager;

	void init();

	void initSystems();
	void initManagers();
	void initGenericUpdaters();

	void processInput();
	void gameLoop();
	void drawVis();
	void drawUi();

	void endFrame();

	GLuint* _frameBufferIDs;
	GLuint* _frameBufferTextureIDs;
	const int _numFrameBuffers = 3;

	int _timeSinceLoadTimerId;

	//uniform updater functions
	float getTimeSinceLoad() { return Vengine::MyTiming::readTimer(_timeSinceLoadTimerId); }
	float getTimeInAudio() { return float(AudioManager::getCurrentSample()) / float(AudioManager::getSampleRate()); }
};

