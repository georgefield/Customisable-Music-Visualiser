#include "UI.h"
#include <Vengine/MyImgui.h>
#include "VisualiserManager.h"
#include "VisualiserShaderManager.h"
#include "FourierTransformManager.h"
#include "Tools.h"
#include <algorithm>

#include "PFDapi.h"

std::vector<std::string> UI::_errorQueue;

UI::UI() :
	_errorMessageTimerId(-1)
{}


void UI::init(Vengine::Window* window, Vengine::InputManager* inputManager) {
	_showUi = true;

	_window = window;
	_inputManager = inputManager;

	_toolbarSizePx = 100;
	_sidebarSizePx = 200;
}


const float ERROR_MESSAGE_DISPLAY_TIME = 3.0f;
void UI::errorMessages() {
	if (_errorQueue.size() == 0) { return; }

	//display oldest queued error for 1 second then pop from vector front
	if (_errorMessageTimerId == -1) {
		Vengine::MyTiming::startTimer(_errorMessageTimerId);
	}
	else if (Vengine::MyTiming::readTimer(_errorMessageTimerId) > ERROR_MESSAGE_DISPLAY_TIME) {
		Vengine::MyTiming::stopTimer(_errorMessageTimerId);
		_errorMessageTimerId = -1;
		_errorQueue.erase(_errorQueue.begin());
		return;
	}

	//create window that only displays error text
	ImGui::SetNextWindowPos(ImVec2(0, _window->getScreenHeight() - 30));

	ImGui::Begin("Message", (bool*)0,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysAutoResize
	);

	ImGui::Text(("Error: " + _errorQueue[0]).c_str());

	ImGui::End();
}


void UI::toolbar() {

	if (!_showUi) {
		return;
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(_window->getScreenWidth(), _toolbarSizePx));

	//create window
	ImGui::Begin("Toolbar", (bool*)0,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar
	);

	//Menu bar
	ImGui::BeginMenuBar();

	if (ImGui::BeginMenu("File")) {
		ImGui::MenuItem("New", NULL, &_new);
		ImGui::MenuItem("Save", NULL, &_save);
		ImGui::MenuItem("Save as", NULL, &_saveAs);
		ImGui::MenuItem("Load", NULL, &_load);
		ImGui::EndMenu();
	}

	if (_save || _saveAs || _load || _new) {
		processFileMenuSelection();
	}

	if (ImGui::BeginMenu("View")) {
		if (ImGui::MenuItem("Toggle UI (tab)")) {
			_showUi = false;
		}

		ImGui::MenuItem("Fullscreen", NULL, &_fullscreen); //todo
		ImGui::EndMenu();
	}


	if (ImGui::BeginMenu("SSBO manager")) {
		ImGui::MenuItem("Show SSBO manager", NULL, &_showSSBOmanager);
		ImGui::EndMenu();
	}

	if (_showSSBOmanager) {
		ssboManagerUi();
	}


	ImGui::EndMenuBar();

	if (ImGui::Button("Add")) {
		ImGui::OpenPopup("add menu");
	}
	if (ImGui::BeginPopup("add menu")) {
		if (ImGui::Selectable("Quad")) {
			SpriteManager::addSprite(glm::vec2(-0.5), glm::vec2(1));
		}
		if (ImGui::Selectable("Circle")) {
			SpriteManager::addSprite(glm::vec2(-0.25), glm::vec2(0.5));
		}
		ImGui::EndPopup();
	}

	//signal processing check boxes
	ImGui::SameLine();
	ImGui::Checkbox("Fourier Transforms UI", &_showFourierTransformUi);

	if (_showFourierTransformUi) {
		fourierTransformsUi();
	}

	ImGui::SameLine();
	ImGui::Checkbox("Note Onset UI", &_showNoteOnsetUi);

	ImGui::SameLine();
	ImGui::Checkbox("Tempo Detection UI", &_showTempoDetectionUi);

	if (_showTempoDetectionUi) {
		//hacked in for now
		ImGui::Begin("Tempo debug", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Predicted tempo: "); ImGui::SameLine();
		ImGui::Text(std::to_string(SignalProcessing::_tempoDetection->getTempoHistory()->newest()).c_str());

		ImGui::Text("Confidence: "); ImGui::SameLine();
		ImGui::Text(std::to_string(SignalProcessing::_tempoDetection->getConfidenceInTempoHistory()->newest()).c_str());
		ImGui::End();
	}

	ImGui::SameLine();
	ImGui::Checkbox("Similarity Measure UI", &_showSimilarityMeasureUi);


	//background colour picker
	static float pickedClearColour[3] = { 0.1, 0.1, 0.1 };
	ImGui::ColorEdit3("Background colour", pickedClearColour, ImGuiColorEditFlags_NoInputs);
	if (ImGui::IsItemEdited()) {
		glClearColor(pickedClearColour[0], pickedClearColour[1], pickedClearColour[2], 1.0f);
	}


	ImGui::End();
}


void UI::sidebar() {
	if (!_showUi) {
		return;
	}

	ImGui::SetNextWindowPos(ImVec2(_window->getScreenWidth() - _sidebarSizePx, _toolbarSizePx));
	ImGui::SetNextWindowSize(ImVec2(_sidebarSizePx, _window->getScreenHeight() - _toolbarSizePx));

	ImGui::Begin("Sidebar", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	//change sprite values from sidebar--

	std::vector<CustomisableSprite*> spritesByDepthOrder = SpriteManager::getDepthSortedSprites();

	ImGui::Text("Sprites");
	ImGui::BeginChild("Sprites", ImVec2(ImGui::GetContentRegionAvail().x, std::min(_window->getScreenHeight() * 0.8f, 500.0f)), true);

	bool sortRequired = false;
	for (int i = spritesByDepthOrder.size() - 1; i >= 0; i--) {
		ImGui::PushID(i);

		ImGui::Separator();

		auto it = spritesByDepthOrder[i];

		//name
		ImGui::Text(it->getName().c_str());

		//change depth
		ImGui::InputFloat("Depth", it->getDepthPtr());
		if (ImGui::IsItemDeactivated()) {
			sortRequired = true; //sort if depth changed
		}

		//show in editor
		ImGui::Checkbox("Show (editor)", it->getShowPtr()); ImGui::SameLine();
		//select sprite
		if (ImGui::Button("Select")) {
			spritesByDepthOrder[i]->setSpriteState(SELECTED);
		}

		//delete
		if (ImGui::Button("Delete")) {
			spritesByDepthOrder[i]->setSpriteState(DELETE_SELF);
		}

		ImGui::PopID();
	}

	ImGui::Separator();

	ImGui::EndChild();

	if (sortRequired) {
		SpriteManager::updateDepthSortedSprites(); //in case any change in depth
	}

	//--



	ImGui::End();
}

void UI::processInput()
{
	if (_inputManager->isKeyPressed(SDLK_TAB)) {
		_showUi = !_showUi;
	}
}


Vengine::Viewport UI::getViewport()
{
	if (!_showUi) {
		return Vengine::Viewport(_window->getScreenWidth(), _window->getScreenHeight());
	}

	int desiredX = _window->getScreenWidth() - _sidebarSizePx;
	int desiredY = _window->getScreenHeight() - _toolbarSizePx;
	bool touchY = ((float)desiredX / (float)desiredY) > _window->getAspectRatio();
	desiredX = (touchY ? desiredY * _window->getAspectRatio() : desiredX);
	desiredY = (touchY ? desiredY : desiredX / _window->getAspectRatio());

	Vengine::Viewport tmp(desiredX, desiredY);
	return tmp;
}

void UI::ssboManagerUi()
{
	//*** SHOW CURRENT BINDINGS ***

	ImGui::Begin("SSBO manager", &_showSSBOmanager, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Set SSBOs:");
	std::vector<int> setBindings;
	VisualiserShaderManager::SSBOs::getSetBindings(setBindings);

	for (auto& it : setBindings) {
		std::string infoStr = VisualiserShaderManager::SSBOs::getSSBOsetterName(it) + " -> SSBO binding " + std::to_string(it);
		ImGui::Text(infoStr.c_str());
		ImGui::SameLine();
		if (ImGui::Button("Unbind")) {
			VisualiserShaderManager::SSBOs::unsetSSBO(it);
		}
	}

	//*** UI FOR CREATING NEW BINDING ***

	//show possible new pairing information
	//choose from setters--
	std::vector<std::string> possibleSSBOsetterNames;
	VisualiserShaderManager::SSBOs::getSSBOsetterNames(possibleSSBOsetterNames);

	std::string settersComboStr = UI::ImGuiComboStringMaker(possibleSSBOsetterNames);

	const char* setterItems = settersComboStr.c_str();
	static int currentSetter = 0;
	ImGui::PushID(0);
	ImGui::Combo("Setters", &currentSetter, setterItems, possibleSSBOsetterNames.size());
	ImGui::PopID();
	//--

	ImGui::Text("Set to binding: ");

	//choose from availiable bindings--
	std::vector<int> availiableBindings;
	VisualiserShaderManager::SSBOs::getAvailiableBindings(availiableBindings);

	std::vector<std::string> strAvailiableBindings;
	for (auto& it : availiableBindings) {
		strAvailiableBindings.push_back(std::to_string(it));
	}

	std::string bindingsComboStr = UI::ImGuiComboStringMaker(strAvailiableBindings);

	const char* bindingItems = bindingsComboStr.c_str();
	static int currentBindingItemIndex = 0;
	ImGui::PushID(1);
	ImGui::Combo("Bindings", &currentBindingItemIndex, bindingItems, strAvailiableBindings.size());
	ImGui::PopID();
	//--

	//button to confirm
	if (ImGui::Button("Confirm")) {
		if (possibleSSBOsetterNames.size() == 0 || availiableBindings.size() == 0) {
			//do nothing
		}
		else {
			VisualiserShaderManager::SSBOs::setSSBO(availiableBindings[currentBindingItemIndex], possibleSSBOsetterNames.at(currentSetter));
			VisualiserShaderManager::SSBOs::getSSBOsetterName(currentBindingItemIndex);
		}
	}

	ImGui::End();
}

void UI::fourierTransformsUi()
{
	ImGui::ShowDemoWindow();
	ImGui::Begin("Fourier Tranforms", &_showFourierTransformUi, ImGuiWindowFlags_AlwaysAutoResize);

	//get id array
	std::vector<int> fourierTransformIds = FourierTransformManager::idArr();

	//*** modify existing transform ***

	//display already created ft and info about them
	for (int i = 0; i < fourierTransformIds.size(); i++) {
		int id = fourierTransformIds[i];

		if (FourierTransformManager::fourierTransformExists(id)) {
			ImGui::PushID(i);

			//name of ft--
			ImGui::Text(("Fourier transform " + std::to_string(id)).c_str());
			//--
			
			//plot low res graph of ft--
			ImGui::PlotLines("##", FourierTransformManager::getFourierTransform(id)->getLowResOutput(), FourierTransformManager::getFourierTransform(id)->getLowResOutputSize());
			//--
			
			//show cutoff information--
			float cutoffLow = FourierTransformManager::getFourierTransform(id)->getCutoffLow();
			ImGui::Text(("Cutoff low: " + std::to_string(cutoffLow)).c_str());
			ImGui::SameLine();
			float cutoffHigh = FourierTransformManager::getFourierTransform(id)->getCutoffHigh();
			ImGui::Text(("Cutoff high: " + std::to_string(cutoffHigh)).c_str());
			float cutoffSmooth = FourierTransformManager::getFourierTransform(id)->getCutoffSmoothFraction();
			ImGui::Text(("Cutoff smooth fraction: " + std::to_string(cutoffSmooth)).c_str());
			//--

			ImGui::Separator();

			//more options for ft--


			//--

			//remove ft
			if (ImGui::Button("Erase")) {
				FourierTransformManager::eraseFourierTransform(id);
			}

			ImGui::Separator();

			ImGui::PopID();
		}
	}

	//*** create another transform ***

	//imgui vars
	static float nextCutoffLow = 0.0f;
	static float nextCutoffHigh = SignalProcessing::getMasterPtr()->_sampleRate / 2.0f;
	static float nextCutoffSmoothFactor = 0.0f;
	ImGui::Text("Ctrl+Click to edit manually");
	ImGui::SliderFloat("Cutoff Hz low", &nextCutoffLow, 0.0f, SignalProcessing::getMasterPtr()->_sampleRate / 2.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Cutoff Hz high", &nextCutoffHigh, 0.0f, SignalProcessing::getMasterPtr()->_sampleRate / 2.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Cutoff smooth fraction", &nextCutoffSmoothFactor, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

	if (ImGui::Button("Create new")) {
		if (nextCutoffHigh < nextCutoffLow) {
			_errorQueue.push_back("Cutoff low cannot be above cutoff high");
		}
		else {
			int id;
			FourierTransformManager::createFourierTransform(id, 1, nextCutoffLow, nextCutoffHigh, nextCutoffSmoothFactor);
		}
	}

	ImGui::ShowDemoWindow();

	ImGui::End();
}

void UI::processFileMenuSelection()
{
	//stop common problem
	if ((_save || _saveAs) && !VisualiserManager::isVisualiserLoaded()) {
		_errorQueue.push_back("Cannot save, no visualiser loaded");
		_save = false;
		_saveAs = false;
	}

	if (_new) {
		static char nameBuf[25];
		bool confirmedName = false;
		if (textInputPrompt("Name of visualiser", nameBuf, 25, confirmedName)) {
			if (confirmedName && !VisualiserManager::createNewVisualiser(nameBuf)) {
				_errorQueue.push_back("Failed to create new visualiser " + std::string(nameBuf));
			}
			nameBuf[0] = NULL; //reset
			_new = false;
		}
	}

	if (_save) {
		if (!VisualiserManager::save()) {
			_errorQueue.push_back("Failed to save");
		}
		_save = false;
		return;
	}

	if (_saveAs) {
		static char nameBuf[25];
		bool confirmedName = false;
		if (textInputPrompt("Save as", nameBuf, 25, confirmedName)) {
			if (confirmedName && !VisualiserManager::saveAsNew(nameBuf)) {
				_errorQueue.push_back("Failed to save as " + std::string(nameBuf));
			}
			nameBuf[0] = NULL; //reset
			_saveAs = false;
		}
	}

	if (_load) {

		std::string visualiserPath = "";
		if (PFDapi::folderChooser("Chooser folder containing the .cfg of the visualiser to load",
			Vengine::IOManager::getProjectDirectory() + "/Visualisers", visualiserPath, false))
		{
			//will always work as cannot go outside of start path
			visualiserPath = visualiserPath.substr(Vengine::IOManager::getProjectDirectory().size() + 1); //+1 to remove '/' at start

			if (!VisualiserManager::loadVisualiser(visualiserPath)) {
				_errorQueue.push_back("Failed to save, likely no visualiser loaded");
			}
		}

		_load = false;
	}
}

bool UI::textInputPrompt(const std::string& message, char* buf, int bufSize, bool& useText)
{
	bool promptNotForceClosed = true;
	bool confirmedName = false;

	//create window
	ImGui::SetNextWindowPos(ImVec2(100, _toolbarSizePx + 30));

	ImGui::Begin(message.c_str(), &promptNotForceClosed,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoCollapse
	);

	ImGui::InputTextWithHint("##", "Enter text here", buf, bufSize);
	ImGui::SameLine();
	if (ImGui::Button("Confirm")) {
		if (buf[0] != NULL) { //string is length > 0
			confirmedName = true;
		}
	}

	ImGui::End();

	useText = confirmedName;
	return !promptNotForceClosed || confirmedName; //return true when user either closes window or confirms name of copy
}

std::string UI::ImGuiComboStringMaker(std::vector<std::string>& options)
{
	std::string retVal = "";
	for (int i = 0; i < options.size(); i++) {
		retVal += options.at(i).c_str();
		retVal += char(NULL); //null character between options
	}

	return retVal;
}

