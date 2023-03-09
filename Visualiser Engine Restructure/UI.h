#pragma once
#include <Vengine/Vengine.h>
#include <glm/glm.hpp>
#include <unordered_map>

#include "SpriteManager.h"
#include "SignalProcessingManager.h"

class UI
{
public:
	UI();
	void init(Vengine::Window* window, Vengine::InputManager* inputManager);

	void toolbar();
	void sidebar();
	void displayErrors();
	void processInput();

	//getters
	bool getShowUi() const { return _showUi; }

	Vengine::Viewport getViewport();

private:

	Vengine::Window* _window;
	Vengine::InputManager* _inputManager;

	bool _showUi;
	//ImGui variables--
	int _toolbarSizePx, _sidebarSizePx;

	//file menu vars
	bool _new;
	bool _save;
	bool _saveAs;
	bool _load;

	//view menu vars
	bool _fullscreen;

	//ssbo menu vars
	bool _showSSBOmanager;
	bool _showUniformManager;
	bool _showImportShaderUi;

	void ssboManagerUi();
	void uniformManagerUi();
	void importShaderUi();

	bool _showGeneralSignalProcessingUi;
	bool _showFourierTransformUi;
	bool _showNoteOnsetUi;
	bool _showTempoDetectionUi;
	bool _showMFCCsUi;
	bool _showSelfSimilarityMatrixUi;

	//calculate
	//--


	//ui sp functions
	void generalSignalProcessingUi();
	void fourierTransformsUi();
	void noteOnsetUi();
	void tempoDetectionUi();
	void mfccUi();
	void selfSimilarityMatrixUi();
	void endSPui();

	//process functions
	void processFileMenuSelection();

	//helper gui functions
	bool textInputPrompt(const std::string& message, char* buf, int bufSize, bool& useText);

	void imguiHistoryPlotter(History<float>* history);

};

