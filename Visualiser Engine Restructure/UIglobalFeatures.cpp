#include "UIglobalFeatures.h"
#include <Vengine/MyTiming.h>

#include "imgui.h"

std::vector<std::string> UIglobalFeatures::_errorQueue;
int UIglobalFeatures::_errorMessageTimerId = -1;
SimMatInfo UIglobalFeatures::_uiSMinfo;
std::vector<std::string> UIglobalFeatures::_syntaxErrorArray;


void UIglobalFeatures::queueError(std::string message)
{
	_errorQueue.push_back(message);
}

const float ERROR_MESSAGE_DISPLAY_TIME = 3.0f;
void UIglobalFeatures::displayErrors() {
	if (_errorQueue.size() == 0) { return; }

	//display oldest queued error for 1 second then pop from vector front
	if (_errorMessageTimerId == -1) {
		Vengine::MyTiming::createTimer(_errorMessageTimerId);
		Vengine::MyTiming::startTimer(_errorMessageTimerId);
	}
	else if (Vengine::MyTiming::readTimer(_errorMessageTimerId) > ERROR_MESSAGE_DISPLAY_TIME) {
		Vengine::MyTiming::resetTimer(_errorMessageTimerId);
		Vengine::MyTiming::startTimer(_errorMessageTimerId);

		_errorQueue.erase(_errorQueue.begin());
		return;
	}

	//create window that only displays error text
	ImGui::SetNextWindowPos(ImVec2(0,ImGui::GetMainViewport()->Size.y - 30));

	ImGui::Begin("Message", (bool*)0,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysAutoResize
	);

	ImGui::Text(("Error: " + _errorQueue[0]).c_str());

	ImGui::End();
}


std::string UIglobalFeatures::ImGuiComboStringMaker(std::vector<std::string>& options)
{
	std::string retVal = "";
	for (int i = 0; i < options.size(); i++) {
		retVal += options.at(i).c_str();
		retVal += char(NULL); //null character between options
	}

	return retVal;
}

bool UIglobalFeatures::ImGuiBetterCombo(std::vector<std::string>& options, int& currentItem, int id)
{
	std::string comboStr = ImGuiComboStringMaker(options);

	ImGui::PushID(id);
	ImGui::Combo("##", &currentItem, comboStr.c_str(), options.size());
	ImGui::PopID();

	return ImGui::IsItemEdited();
}

bool UIglobalFeatures::ImGuiBetterCombo(std::vector<int>& options, int& currentItem, int id)
{
	std::string comboStr = "";
	for (int i = 0; i < options.size(); i++) {
		comboStr += std::to_string(options.at(i));
		comboStr += char(NULL); //null character between options
	}

	ImGui::PushID(id);
	ImGui::Combo("##", &currentItem, comboStr.c_str(), options.size());
	ImGui::PopID();

	return ImGui::IsItemEdited();
}

void UIglobalFeatures::addSyntaxErrorToWindow(const std::string& error) {
	queueError("Error compiling shader, check 'Show shaders errors' under 'Shader managing' for details");
	_syntaxErrorArray.push_back(error);
}
void UIglobalFeatures::clearSyntaxErrorWindow() {
	_syntaxErrorArray.clear();
}
std::vector<std::string>* UIglobalFeatures::getSyntaxErrorArray() {
	return &_syntaxErrorArray;
}