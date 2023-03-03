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
	void errorMessages();
	void processInput();

	//getters
	bool getShowUi() const { return _showUi; }

	Vengine::Viewport getViewport();

	//can push to error queue anywhere 'UI.h' is included
	static std::vector<std::string> _errorQueue;

	static std::string ImGuiComboStringMaker(std::vector<std::string>& options);
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
	bool _showSimilarityMeasureUi;

	//calculate
	//--

	int _errorMessageTimerId;

	//ui functions
	void generalSignalProcessingUi();
	void fourierTransformsUi();
	void noteOnsetUi();
	void tempoDetectionUi();

	//process functions
	void processFileMenuSelection();

	//helper gui functions
	bool textInputPrompt(const std::string& message, char* buf, int bufSize, bool& useText);

};

