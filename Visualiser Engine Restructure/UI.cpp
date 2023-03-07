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


	if (ImGui::BeginMenu("Shader Managing")) {
		ImGui::MenuItem("Shader Storage Buffer Objects (SSBOs)", NULL, &_showSSBOmanager);
		ImGui::MenuItem("Uniforms", NULL, &_showUniformManager);
		ImGui::MenuItem("Import shader", NULL, &_showImportShaderUi);
		ImGui::EndMenu();
	}

	if (_showSSBOmanager) {
		ssboManagerUi();
	}
	if (_showUniformManager) {
		uniformManagerUi();
	}
	if (_showImportShaderUi) {
		importShaderUi();
	}

	if (ImGui::BeginMenu("Signal Processing")) {
		ImGui::MenuItem("General", NULL, &_showGeneralSignalProcessingUi);
		ImGui::MenuItem("Frequency bands", NULL, &_showFourierTransformUi);
		ImGui::MenuItem("Note onset detection", NULL, &_showNoteOnsetUi);
		ImGui::MenuItem("Tempo detection", NULL, &_showTempoDetectionUi);
		ImGui::MenuItem("MFCCs", NULL, &_showMFCCsUi);
		ImGui::MenuItem("Self Similarity Matrix", NULL, &_showSelfSimilarityMatrixUi);
		//add rest
		ImGui::EndMenu();
	}

	if (_showGeneralSignalProcessingUi) {
		generalSignalProcessingUi();
	}
	if (_showFourierTransformUi) {
		fourierTransformsUi();
	}
	if (_showNoteOnsetUi) {
		noteOnsetUi();
	}
	if (_showTempoDetectionUi) {
		tempoDetectionUi();
	}
	if (_showMFCCsUi) {
		mfccUi();
	}
	if (_showSelfSimilarityMatrixUi) {
		selfSimilarityMatrixUi();
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


//*** SHADER UI ***

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

void UI::uniformManagerUi()
{
	//*** CHOOSE SHADER TO EDIT UNIFORMS OF ***

	ImGui::Begin("Uniform manager", &_showUniformManager, ImGuiWindowFlags_AlwaysAutoResize);

	static std::vector<std::string> shaderFileNames;
	static bool firstOpen = true;

	if (firstOpen || _inputManager->isKeyPressOrMouseClickThisFrame()) {
		Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), shaderFileNames, true, ".frag");
		firstOpen = false;
	}

	ImGui::Text("Select shader: ");

	std::string shaderComboStr = UI::ImGuiComboStringMaker(shaderFileNames);

	const char* shaderItems = shaderComboStr.c_str();
	static int currentShader = 0;
	ImGui::PushID(2);
	ImGui::Combo("Shaders", &currentShader, shaderItems, shaderFileNames.size());
	ImGui::PopID();

	if (shaderFileNames.size() == 0) {
		ImGui::Text("No shaders in visualiser shaders folder");
		ImGui::End();
		return;
	}

	VisualiserShader* chosenShader = VisualiserShaderManager::getShader(VisualiserManager::shadersFolder() + "/" + shaderFileNames.at(currentShader));

	ImGui::Separator();


	//*** DISPLAY CURRENT SET UNIFORMS ***

	std::vector<std::string> setUniformNames;
	chosenShader->getSetUniformNames(setUniformNames);

	//display pairings
	for (auto& it : setUniformNames) {
		std::string info = chosenShader->getUniformSetterName(it) + " -> " + it;

		ImGui::Text(info.c_str()); ImGui::SameLine();
		if (ImGui::Button("Erase")) {
			chosenShader->eraseSetterUniformPair(it);
		}
	}

	if (setUniformNames.size() > 0) {
		ImGui::Separator();
	}


	//*** UI FOR CREATING NEW PAIRINGS OF UNIFORMS AND SETTERS FOR SHADER ***

	//show possible new pairing information
	std::vector<std::string> unsetUniformNames; //any unset uniform
	chosenShader->getUnsetUniformNames(unsetUniformNames);

	if (unsetUniformNames.size() + setUniformNames.size() > 0) {

		//choose from uniforms to set--
		std::string uniformComboStr = UI::ImGuiComboStringMaker(unsetUniformNames);

		const char* uniformItems = uniformComboStr.c_str();
		static int currentUniform = 0;
		ImGui::PushID(0);
		ImGui::Combo("Uniforms", &currentUniform, uniformItems, unsetUniformNames.size());
		ImGui::PopID();
		//--

		ImGui::Text("To be set to:");

		//choose from valid possible uniform setters--
		std::vector<std::string> possibleUniformSetterFunctionNames; //can be paired with any function

		if (unsetUniformNames.size() != 0 && chosenShader->getUniformType(unsetUniformNames.at(currentUniform)) == GL_INT) {
			VisualiserShaderManager::Uniforms::getIntUniformSetterNames(possibleUniformSetterFunctionNames);
		}
		else if (unsetUniformNames.size() != 0 && chosenShader->getUniformType(unsetUniformNames.at(currentUniform)) == GL_FLOAT) {
			VisualiserShaderManager::Uniforms::getFloatUniformSetterNames(possibleUniformSetterFunctionNames);
		}

		std::string uniformSetterComboStr = UI::ImGuiComboStringMaker(possibleUniformSetterFunctionNames);

		const char* uniformSetterItems = uniformSetterComboStr.c_str();
		static int currentUniformSetter = 0;
		ImGui::PushID(1);
		ImGui::Combo("Setters", &currentUniformSetter, uniformSetterItems, possibleUniformSetterFunctionNames.size());
		ImGui::PopID();
		//--

		//button to confirm
		if (ImGui::Button("Confirm")) {
			if (possibleUniformSetterFunctionNames.size() == 0 || unsetUniformNames.size() == 0) {
				//do nothing
			}
			else if (chosenShader->getUniformType(unsetUniformNames.at(currentUniform)) == GL_INT) {
				chosenShader->initSetterUniformPair(unsetUniformNames.at(currentUniform), VisualiserShaderManager::Uniforms::getIntUniformSetter(possibleUniformSetterFunctionNames.at(currentUniformSetter)));
			}
			else if (chosenShader->getUniformType(unsetUniformNames.at(currentUniform)) == GL_FLOAT) {
				chosenShader->initSetterUniformPair(unsetUniformNames.at(currentUniform), VisualiserShaderManager::Uniforms::getFloatUniformSetter(possibleUniformSetterFunctionNames.at(currentUniformSetter)));
			}
		}
	}
	else {
		ImGui::Text("No uniforms in shader");
	}

	ImGui::End();
}

void UI::importShaderUi()
{
	std::string chosenFile = "";
	if (PFDapi::fileChooser("Choose shader to add", Vengine::IOManager::getProjectDirectory(), chosenFile, { "Fragment Files (.frag)", "*.frag" }, true)) {
		VisualiserShaderManager::getShader(chosenFile);
	}
	else {
		_errorQueue.push_back("Could not import shader " + chosenFile);
	}

	_showImportShaderUi = false;
}

//***


//*** SIGNAL PROCESSING UIs ***

void UI::generalSignalProcessingUi()
{
	ImGui::Begin("General", &_showGeneralSignalProcessingUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Fourier Transforms", &SPvars::UI::_computeFourierTransform);
	ImGui::Checkbox("Compute RMS", &SPvars::UI::_computeRms);
	ImGui::Checkbox("Compute Note Onset", &SPvars::UI::_computeNoteOnset);
	ImGui::Checkbox("Compute Tempo Detection", &SPvars::UI::_computeTempoDetection);
	ImGui::Checkbox("Compute MFCCs", &SPvars::UI::_computeMFCCs);
	ImGui::Checkbox("Compute Self Similarity Matrix", &SPvars::UI::_computeSimilarityMatrix);

	std::string fpsInfo = "FPS: " + std::to_string(int(Vengine::MyTiming::getFPS()));
	ImGui::Text(fpsInfo.c_str());

	if (ImGui::Button("Restart signal processing")) {
		SignalProcessingManager::restart();
	}
	ImGui::End();
}

void UI::fourierTransformsUi()
{
	ImGui::Begin("Fourier Tranforms", &_showFourierTransformUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Fourier Transforms", &SPvars::UI::_computeFourierTransform);

	//get id array
	std::vector<int> fourierTransformIds = FourierTransformManager::idArr();

	//*** modify existing transform ***

	//display already created ft and info about them
	for (int i = 0; i < fourierTransformIds.size(); i++) {
		int id = fourierTransformIds[i];
		std::string ftName = "Fourier Transform " + std::to_string(id);
		if (ImGui::CollapsingHeader(ftName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

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
	static float nextCutoffHigh = SignalProcessingManager::getMasterPtr()->_sampleRate / 2.0f;
	static float nextCutoffSmoothFactor = 0.0f;
	ImGui::Text("Ctrl+Click to edit manually");
	ImGui::SliderFloat("Cutoff Hz low", &nextCutoffLow, 0.0f, SignalProcessingManager::getMasterPtr()->nyquist(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Cutoff Hz high", &nextCutoffHigh, 0.0f, SignalProcessingManager::getMasterPtr()->nyquist(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
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


	ImGui::End();
}

void UI::noteOnsetUi()
{
	ImGui::ShowDemoWindow();

	ImGui::Begin("Note Onset", &_showNoteOnsetUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute note onset", &SPvars::UI::_computeNoteOnset);

	ImGui::Text("Onset detection function:");
	ImGui::RadioButton("Derivative of log energy", &SPvars::UI::_onsetDetectionFunctionEnum, 0);
	ImGui::RadioButton("Banded derivative of log energy", &SPvars::UI::_onsetDetectionFunctionEnum, 1);
	ImGui::RadioButton("Spectral distance", &SPvars::UI::_onsetDetectionFunctionEnum, 2);
	ImGui::RadioButton("Similarity matrix", &SPvars::UI::_onsetDetectionFunctionEnum, 3);
	ImGui::RadioButton("Combination", &SPvars::UI::_onsetDetectionFunctionEnum, 4);


	ImGui::Checkbox("Convolve onset detection", &SPvars::UI::_convolveOnsetDetection);

	imguiHistoryPlotter(SignalProcessingManager::_noteOnset->getOnsetHistory(SPvars::UI::_convolveOnsetDetection));
	imguiHistoryPlotter(SignalProcessingManager::_noteOnset->getDisplayPeaks());

	ImGui::End();
}

void UI::tempoDetectionUi()
{
	ImGui::Begin("Tempo", &_showTempoDetectionUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Tempo", &SPvars::UI::_computeTempoDetection);

	std::string tempo = "Tempo guess: " + std::to_string(SignalProcessingManager::_tempoDetection->getTempo());
	ImGui::Text(tempo.c_str());
	float confidence = SignalProcessingManager::_tempoDetection->getConfidenceInTempo();
	std::string confidenceStr = "Confidence: " + std::to_string(confidence);
	ImGui::Text(confidenceStr.c_str());
	ImGui::SameLine();
	if (confidence < 0.3) {
		ImGui::Text(" (No idea)");
	}
	else if (confidence < 0.5) {
		ImGui::Text(" (Unsure)");
	}
	else if (confidence < 0.66) {
		ImGui::Text(" (Somewhat confident)");
	}
	else {
		ImGui::Text(" (Confident)");
	}

	std::string timeToNextBeat = "Time to next beat: " + std::to_string(SignalProcessingManager::_tempoDetection->getTimeToNextBeat());
	ImGui::Text(timeToNextBeat.c_str());
	std::string timeSinceLastBeat = "Time since last beat: " + std::to_string(SignalProcessingManager::_tempoDetection->getTimeSinceLastBeat());
	ImGui::Text(timeSinceLastBeat.c_str());

	static bool showDebug = false;
	ImGui::Checkbox("Debug", &showDebug);
	if (showDebug) {
		ImGui::BeginChild("debug", ImVec2(500, 350), true);
		std::vector < std::string> debugInfo;
		SignalProcessingManager::_tempoDetection->getDebugInfo(debugInfo);
		for (auto& it : debugInfo) {
			ImGui::Text(it.c_str());
		}
		ImGui::EndChild();
	}

	ImGui::End();
}

void UI::mfccUi()
{
	ImGui::Begin("MFCCs", &_showMFCCsUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute MFCCs", &SPvars::UI::_computeMFCCs);

	ImGui::PlotHistogram("Mel band energies", SignalProcessingManager::_mfccs->getBandEnergies(), SignalProcessingManager::_mfccs->getNumMelBands());
	ImGui::PlotHistogram("Mel spectrogram", SignalProcessingManager::_mfccs->getMelSpectrogram(), SignalProcessingManager::_mfccs->getNumMelBands());
	ImGui::PlotHistogram("MFCCs", SignalProcessingManager::_mfccs->getMfccs(), SignalProcessingManager::_mfccs->getNumMelBands());

	ImGui::End();
}

void UI::selfSimilarityMatrixUi()
{
	ImGui::Begin("Self Similarity Matrix", &_showSelfSimilarityMatrixUi, ImGuiWindowFlags_AlwaysAutoResize);

	//whether to calculate & which one to calculate (future or real time)--
	static int realTimeOrFuture = 0;
	ImGui::Text("Link to:");
	ImGui::RadioButton("Real time", &realTimeOrFuture, 0); ImGui::SameLine();
	ImGui::RadioButton("Future (recommended)", &realTimeOrFuture, 1);

	bool isRealTime = (realTimeOrFuture == 0);
	bool isFuture = (realTimeOrFuture == 1);

	ImGui::Checkbox("Compute Self Similarity Matrix", &SPvars::UI::_computeSimilarityMatrix);
	//--

	//track if settings changed, linking function called if confirm pressed and reset similarity matrix--
	static bool changedMatrixSettings = false;
	static std::function<void()> linkingFunction;
	//--

	if (changedMatrixSettings) {
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "--Need to confirm--");
	}
	else {
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "--Confirmed, is calculating--");
	}


	//*** matrix settings***

	ImGui::BeginChild("Matrix settings", ImVec2(350, 400), true);

	//matrix size--
	static int matrixSizeTmp = SPvars::UI::_nextSimilarityMatrixSize;
	ImGui::SliderInt("Matrix Size", &matrixSizeTmp, 0, 1000);

	if (matrixSizeTmp != SPvars::UI::_nextSimilarityMatrixSize) {
		SPvars::UI::_nextSimilarityMatrixSize = matrixSizeTmp;
		changedMatrixSettings = true;
	}
	//--

	//fast or slow texture creation--
	static int fastOrSlow = SPvars::UI::_fastSimilarityMatrixTexture;
	ImGui::Text("Texture creation type:");
	ImGui::RadioButton("Fast", &fastOrSlow, 1); ImGui::SameLine();
	ImGui::RadioButton("Slow", &fastOrSlow, 0);

	if (fastOrSlow != SPvars::UI::_fastSimilarityMatrixTexture) {
		SPvars::UI::_fastSimilarityMatrixTexture = fastOrSlow;
		changedMatrixSettings = true;
	}
	//--

	//link to options--
	static int linkTo = 0;
	ImGui::Text("Link to:");
	ImGui::RadioButton("MFCCs", &linkTo, 0);
	ImGui::RadioButton("Mel band energies", &linkTo, 1);
	ImGui::RadioButton("Mel spectrogram", &linkTo, 2);
	ImGui::RadioButton("Fourier transform", &linkTo, 3);
	//--



	if (linkTo == 0) { // link to mfccs vv
		ImGui::Separator();

		ImGui::Text("MFCCs coefficients: ");
		static int low = 4;
		ImGui::PushID(0);
		ImGui::SliderInt("##", &low, 1, SPvars::Const::_numMelBands);
		ImGui::PopID();

		ImGui::Text("to");

		static int high = 13;
		ImGui::PushID(1);
		ImGui::SliderInt("##", &high, 1, SPvars::Const::_numMelBands);
		ImGui::PopID();
		//|= to stop it overriding changedMatrixSettings that was set to true elsewhere
		changedMatrixSettings |= !SignalProcessingManager::_similarityMatrix->isLinkInfoTheSame(low, high);
		if (changedMatrixSettings) {
			linkingFunction = std::bind(&SimilarityMatrixHandler::linkToMFCCs, SignalProcessingManager::_similarityMatrix, low, high);
		}
	}
	if (linkTo == 1) { // link to mel band energies (real time only) vv
		changedMatrixSettings |= SignalProcessingManager::_similarityMatrix->isLinkedTo() != MelBandEnergies;
		if (changedMatrixSettings) {
			linkingFunction = std::bind(&SimilarityMatrixHandler::linkToMelBandEnergies, SignalProcessingManager::_similarityMatrix);
		}
	}
	if (linkTo == 2) {	// link to mel spectrogram (real time only) vv
		changedMatrixSettings |= SignalProcessingManager::_similarityMatrix->isLinkedTo() != MelSpectrogram;
		if (changedMatrixSettings) {
			linkingFunction = std::bind(&SimilarityMatrixHandler::linkToMelSpectrogram, SignalProcessingManager::_similarityMatrix);
		}
	}
	if (linkTo == 3) {	// link to fourier transform vv
		static float nextCutoffLow = 0.0f;
		static float nextCutoffHigh = SignalProcessingManager::getMasterPtr()->nyquist();
		static float nextCutoffSmoothFactor = 0.0f;

		ImGui::Text("Ctrl+Click to edit manually");
		ImGui::SliderFloat("Cutoff Hz low", &nextCutoffLow, 0.0f, SignalProcessingManager::getMasterPtr()->_sampleRate / 2.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Cutoff Hz high", &nextCutoffHigh, 0.0f, SignalProcessingManager::getMasterPtr()->_sampleRate / 2.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Cutoff smooth fraction", &nextCutoffSmoothFactor, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

		changedMatrixSettings |= !SignalProcessingManager::_similarityMatrix->isLinkInfoTheSame(nextCutoffLow, nextCutoffHigh, nextCutoffSmoothFactor);
		if (changedMatrixSettings) {
			linkingFunction = std::bind(&SimilarityMatrixHandler::linkToFourierTransform, SignalProcessingManager::_similarityMatrix, nextCutoffLow, nextCutoffHigh, nextCutoffSmoothFactor);
		}
	}

	//measure type of similarity matrix--
	static int measureTypeEnum = SPvars::UI::_matrixMeasureEnum;
	ImGui::Text("Matrix measure type:");
	ImGui::RadioButton("Similarity", &measureTypeEnum, 0); ImGui::SameLine();
	ImGui::RadioButton("Percussion", &measureTypeEnum, 1);

	if (measureTypeEnum != SPvars::UI::_matrixMeasureEnum) {
		SPvars::UI::_matrixMeasureEnum = measureTypeEnum;
	}
	//--
	ImGui::SliderFloat("Texture contrast", &SPvars::UI::_similarityMatrixTextureContrastFactor, 1, 100, "%.3f", ImGuiSliderFlags_Logarithmic);

	if (changedMatrixSettings && ImGui::Button("Confirm", ImVec2(80, 25))) {
		SignalProcessingManager::_similarityMatrix->reInit(SPvars::UI::_nextSimilarityMatrixSize);
		linkingFunction(); //even if no change in linking function must relink after resize. Because this is static will remember last function linked to
		changedMatrixSettings = false;
	}

	ImGui::EndChild();
	//***

	imguiHistoryPlotter(SignalProcessingManager::_similarityMatrix->matrix.getSimilarityMeasureHistory());

	ImGui::End();
}

//***

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

void UI::imguiHistoryPlotter(History<float>* history)
{
	ImGui::PlotLines("##", history->dataStartPtr(), history->totalSize(), history->firstPartOffset());
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

