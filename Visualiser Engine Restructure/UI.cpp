#include "UI.h"
#include <Vengine/MyImgui.h>
#include "VisualiserManager.h"
#include "Tools.h"
#include <algorithm>

#include <portable-file-dialogs.h>

UI::UI():
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
	else if (Vengine::MyTiming::readTimer(_errorMessageTimerId) > ERROR_MESSAGE_DISPLAY_TIME){
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
		ImGui::MenuItem("Save", NULL, &_save);
		ImGui::MenuItem("Save as", NULL, &_saveAs);
		ImGui::MenuItem("Load", NULL, &_load);
		ImGui::EndMenu();
	}

	if (_save || _saveAs || _load) {
		fileMenuAction();
	}

	if (ImGui::BeginMenu("View")) {
		if (ImGui::MenuItem("Toggle UI (tab)")) {
			_showUi = false;
		}

		ImGui::MenuItem("Fullscreen", NULL, &_fullscreen);
		ImGui::EndMenu();
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
	ImGui::BeginChild("Sprites", ImVec2(ImGui::GetContentRegionAvail().x, min(_window->getScreenHeight() * 0.8f, 500.0f)), true);

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

void UI::fileMenuAction()
{
	if (_save) {
		if (!VisualiserManager::save()) {
			_errorQueue.push_back("Failed to save, likely no visualiser loaded");
		}
		_save = false;
		return;
	}

	if (_saveAs) {
		static char nameBuf[25];
		nameBuf[0] = NULL;
		if (textInputPrompt("Save as", nameBuf)) {
			if (!VisualiserManager::saveAsNew(nameBuf)) {
				_errorQueue.push_back("Failed to save, likely no visualiser loaded");
			}
			_saveAs = false;
		}
	}

	if (_load) {
		std::string visualiserPath = "";
		if (folderChooser(Vengine::IOManager::getProjectDirectory() + "/Visualisers", visualiserPath, false)) {
			//will always work as cannot go outside of start path
			visualiserPath = visualiserPath.substr(Vengine::IOManager::getProjectDirectory().size());

			if (!VisualiserManager::loadVisualiser(visualiserPath)) {
				_errorQueue.push_back("Failed to save, likely no visualiser loaded");
			}
		}

		_load = false;
	}
}

bool UI::textInputPrompt(const std::string& message, char* out)
{
	bool promptNotForceClosed = true;
	bool confirmedName = false;

	//create window
	ImGui::SetNextWindowPos(ImVec2(100,100));
	ImGui::SetNextWindowSize(ImVec2(300, 200));

	ImGui::Begin(message.c_str(), &promptNotForceClosed,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_MenuBar
	);

	ImGui::InputTextWithHint("##", "Enter text here", out, IM_ARRAYSIZE(out));
	ImGui::SameLine();
	if (ImGui::Button("Confirm")) {
		if (out[0] != NULL) { //string is length > 0
			confirmedName = true;
		}
	}

	ImGui::End();
	return promptNotForceClosed;
}

bool UI::folderChooser(std::string startPath, std::string& out, bool loadFromOutsideStartPath)
{
	// Check that a backend is available
	if (!pfd::settings::available())
	{
		_errorQueue.push_back("Portable File Dialogs are not available on this platform");
		return false;
	}

	std::replace(startPath.begin(), startPath.end(), '/', '\\');

	// Directory selection
	std::string folderPath = pfd::select_folder("Select visualiser directory", startPath).result();

	if (!loadFromOutsideStartPath && folderPath.substr(0, startPath.size()) != startPath) {
		_errorQueue.push_back("Cannot load from outside " + startPath);
		return false;
	}

	out = folderPath;

	return true;
}


