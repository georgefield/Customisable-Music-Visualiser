#include "UI.h"
#include <Vengine/MyImgui.h>
#include "VisualiserManager.h"
#include "VisualiserShaderManager.h"
#include "FourierTransformManager.h"
#include "Tools.h"
#include <algorithm>
#include "UIglobalFeatures.h"

#include "PFDapi.h"

#define FLOAT_MAX 3.4028235E38F


UI::UI()
{}


void UI::init(Vengine::Window* window, Vengine::InputManager* inputManager) {
	_showUi = true;
	_showInfoBoxes = true;

	_window = window;
	_inputManager = inputManager;

	_toolbarSizePx = 100;
	_sidebarSizePx = 200;
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
		ImGui::MenuItem("Shader variables", NULL, &_showShaderVariablesUi);
		ImGui::MenuItem("Import shader", NULL, &_showImportShaderUi);
		ImGui::MenuItem("Create Shader", NULL, &_showCreateShaderUi);
		ImGui::EndMenu();
	}

	if (_showInfoBoxes) {
		if (_showShaderVariablesUi)
			shaderVariablesUi();
		if (_showImportShaderUi)
			importShaderUi();
		if (_showCreateShaderUi)
			createShaderUi();
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

	if (_showInfoBoxes) {
		if (_showGeneralSignalProcessingUi)
			generalSignalProcessingUi();
		if (_showFourierTransformUi)
			fourierTransformsUi();
		if (_showNoteOnsetUi)
			noteOnsetUi();
		if (_showTempoDetectionUi)
			tempoDetectionUi();
		if (_showMFCCsUi)
			mfccUi();
		if (_showSelfSimilarityMatrixUi)
			selfSimilarityMatrixUi();
	}

	endSPui(); //resets vars that were set from signal processing to notify ui elements

	ImGui::EndMenuBar();

	//add sprite--
	ImGui::BeginChild("add button child", ImVec2(ImGui::GetContentRegionAvail().y *2, ImGui::GetContentRegionAvail().y), false);

	if (ImGui::Button("Add sprite", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {
		ImGui::OpenPopup("add menu");
	}
	if (ImGui::BeginPopup("add menu")) {
		if (ImGui::Selectable("Quad"))
			SpriteManager::addSprite(Vengine::Mod_Quad, glm::vec2(-0.5), glm::vec2(1));
		if (ImGui::Selectable("Triangle"))
			SpriteManager::addSprite(Vengine::Mod_Triangle, glm::vec2(-0.5), glm::vec2(1));
		if (ImGui::Selectable("Circle"))
			SpriteManager::addSprite(Vengine::Mod_Circle, glm::vec2(-0.5), glm::vec2(1));
		if (ImGui::Selectable("Ring"))
			SpriteManager::addSprite(Vengine::Mod_Ring, glm::vec2(-0.5), glm::vec2(1));

		ImGui::EndPopup();
	}

	ImGui::EndChild();
	//--

	ImGui::SameLine();

	//audio playback ui--
	ImGui::BeginChild("playback child", ImVec2(250, ImGui::GetContentRegionAvail().y), false);

	//load song
	if (ImGui::Button("Load song")) {
		std::string chosenAudio;
		PFDapi::fileChooser("Choose audio", Vengine::IOManager::getProjectDirectory(), chosenAudio, { "Audio files", "*.wav *.mp3 *.flac" }, true);
		AudioManager::load(chosenAudio);
	}

	//music name
	std::string audioInfo = "\"" + AudioManager::getAudioFileName() + "\"";
	ImGui::Text(audioInfo.c_str());

	//audio scrubbing
	ImGui::SetNextItemWidth(100);

	static int scrubber = 0;
	ImGui::DragInt("##", &scrubber, 5000.0f, 0, AudioManager::getNumSamples(), "%d", ImGuiSliderFlags_AlwaysClamp);
	if (ImGui::IsItemEdited()) {
		AudioManager::seekToSample(scrubber);
		SPvars._nextCalculationSample = scrubber;
	}
	if (!ImGui::IsItemActive() && AudioManager::isAudioPlaying()) {
		scrubber = AudioManager::getCurrentSample();
	}

	ImGui::SameLine();

	//audio play/pause
	if (!AudioManager::isAudioPlaying() && ImGui::ArrowButton("Play", ImGuiDir_Right)) {
		AudioManager::play();
	}
	else if (AudioManager::isAudioPlaying() && ImGui::ArrowButton("Pause", ImGuiDir_Square)) {
		AudioManager::pause();
	}

	ImGui::SameLine();

	//music info
	ImGui::Text(AudioManager::getAudioTimeInfoStr(scrubber).c_str());


	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("db meter", ImVec2(150, ImGui::GetContentRegionAvail().y));

	float logAmplitude = SignalProcessingManager::getMasterPtr()->getPeakAmplitudeDb() + 12;
	ImGui::PlotHistogram("0 db\n-3 db\n-6 db\n-9 db", &logAmplitude, 1, 0, "##", 0, 13, ImVec2(20, 65));

	ImGui::SameLine();

	static float volume = 1; 
	ImGui::VSliderFloat("Volume", ImVec2(30, 65), &volume, 0, 1, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	AudioManager::setVolume(volume);

	ImGui::EndChild();
	//--

	ImGui::SameLine();



	//background colour picker--
	static float pickedClearColour[3] = { 0.1, 0.1, 0.1 };
	ImGui::ColorEdit3("Background colour", pickedClearColour, ImGuiColorEditFlags_NoInputs);
	if (ImGui::IsItemEdited()) {
		glClearColor(pickedClearColour[0], pickedClearColour[1], pickedClearColour[2], 1.0f);
	}
	//--

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
		ImGui::Checkbox("Show (editor)", it->getShowInEditorPtr()); ImGui::SameLine();
		//select sprite
		if (ImGui::Button("Select")) {
			SpriteManager::deselectCurrent();
			spritesByDepthOrder[it->id]->setIfSelected(true);
		}

		//delete
		if (ImGui::Button("Delete")) {
			SpriteManager::deselectCurrent();
			SpriteManager::deleteSprite(it->id);
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

void UI::displayErrors()
{
	UIglobalFeatures::displayErrors();
}

void UI::processInput()
{
	if (_inputManager->isKeyPressed(SDLK_TAB)) {
		_showUi = !_showUi;
	}

	if (_inputManager->isKeyDown(SDLK_LCTRL) && _inputManager->isKeyPressed(SDLK_x)) {
		_showInfoBoxes = !_showInfoBoxes;
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

void UI::shaderVariablesUi()
{
	//*** SHOW CURRENT BINDINGS ***

	ImGui::Begin("Shader variables", &_showShaderVariablesUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("These are the variable that you can use in a .visfrag shader");

	if (ImGui::CollapsingHeader("Arrays")) {
		std::vector<std::string> SSBOnames;
		VisualiserShaderManager::SSBOs::getAllSSBOnames(SSBOnames);
		std::sort(SSBOnames.begin(), SSBOnames.end()); //sort alphabetically

		for (auto& it : SSBOnames) {
			std::string infoStr = it + " = " + VisualiserShaderManager::SSBOs::getSSBOstatus(it);
			ImGui::Text(infoStr.c_str());
		}

	}

	if (ImGui::CollapsingHeader("Values")) {
		std::vector<std::string> uniformNames;
		VisualiserShaderManager::Uniforms::getAllUniformNames(uniformNames);
		std::sort(uniformNames.begin(), uniformNames.end()); //sort alphabetically

		for (auto& it : uniformNames) {
			std::string infoStr = it + " = ";

			GLenum type;
			float value = VisualiserShaderManager::Uniforms::getUniformValue(it, &type);
			if (type == GL_INT)
				infoStr += std::to_string(int(value));
			else
				infoStr += std::to_string(value);
			ImGui::Text(infoStr.c_str());
		}
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
		UIglobalFeatures::queueError("Could not import shader " + chosenFile);
	}

	_showImportShaderUi = false;
}

void UI::createShaderUi()
{
	static char shaderName[25] = "";
	if (textInputPrompt("Shader name:", shaderName, 25, _showCreateShaderUi)) {
		if (!VisualiserShaderManager::createShader(shaderName)) {
			UIglobalFeatures::queueError("Could not create new shader file");
		}
		shaderName[0] = NULL;
	}
}

//***


//*** SIGNAL PROCESSING UIs ***

void UI::generalSignalProcessingUi()
{
	ImGui::Begin("General", &_showGeneralSignalProcessingUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Fourier Transforms", &SPvars._computeFourierTransform);
	ImGui::Checkbox("Compute RMS", &SPvars._computeRms);
	ImGui::Checkbox("Compute Note Onset", &SPvars._computeNoteOnset);
	ImGui::Checkbox("Compute Tempo Detection", &SPvars._computeTempoDetection);
	ImGui::Checkbox("Compute MFCCs", &SPvars._computeMFCCs);
	ImGui::Checkbox("Compute Self Similarity Matrix", &SPvars._computeSimilarityMatrix);

	ImGui::Separator();

	std::string fpsInfo = "FPS: " + std::to_string(int(Vengine::MyTiming::getFPS()));
	ImGui::Text(fpsInfo.c_str());

	std::string cpsInfo = "Audio calculations per second (CPS): " + std::to_string(int(SPvars._desiredCPS));
	ImGui::Text(cpsInfo.c_str());
	ImGui::Text("Can auto decrease if calculations fall behind");

	static float CPS = SPvars._desiredCPS;
	if (SPvars._wasCPSautoDecreased) {
		CPS = SPvars._desiredCPS;
	}

	ImGui::SliderFloat("CPS", &CPS, 10, 120, "%.1f");
	if (CPS != SPvars._desiredCPS && ImGui::Button("Set CPS")) {
		SPvars._desiredCPS = CPS;
		SignalProcessingManager::reset();
	}

	//add ability to change fourier transform window size; use combo to choose from 1024 samples to 16192

	if (ImGui::Button("Reset signal processing")) {
		SignalProcessingManager::reset();
	}

	ImGui::Separator();

	if (AudioManager::isAudioLoaded()) {
		std::string info = "Sample rate: " + std::to_string(AudioManager::getSampleRate()) + ", Current sample: " + std::to_string(AudioManager::getCurrentSample()) + " / " + std::to_string(AudioManager::getNumSamples());
		ImGui::Text(info.c_str());
		ImGui::PlotLines("data", &(AudioManager::getSampleData()[std::max(0, AudioManager::getCurrentSample() - AudioManager::getSampleRate())]), AudioManager::getSampleRate(), 0, 0, -1, 1, ImVec2(350, 40));
	}
	else {
		ImGui::Text("No audio loaded");
	}

	ImGui::End();
}

void UI::fourierTransformsUi()
{
	ImGui::Begin("Fourier Tranforms", &_showFourierTransformUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Fourier Transforms", &SPvars._computeFourierTransform);

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
			UIglobalFeatures::queueError("Cutoff low cannot be above cutoff high");
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

	ImGui::Checkbox("Compute note onset", &SPvars._computeNoteOnset);

	ImGui::Text("Onset detection function:");
	ImGui::RadioButton("Energy", &SPvars._onsetDetectionFunctionEnum, 0);
	ImGui::RadioButton("Derivative of log energy", &SPvars._onsetDetectionFunctionEnum, 1);
	ImGui::RadioButton("Banded derivative of log energy", &SPvars._onsetDetectionFunctionEnum, 2);
	ImGui::RadioButton("Spectral distance", &SPvars._onsetDetectionFunctionEnum, 3);
	ImGui::RadioButton("Spectral distance high freq. weighted", &SPvars._onsetDetectionFunctionEnum, 4);
	ImGui::RadioButton("Similarity matrix", &SPvars._onsetDetectionFunctionEnum, 5);
	ImGui::RadioButton("Combination (fast)", &SPvars._onsetDetectionFunctionEnum, 6);
	ImGui::RadioButton("Combination", &SPvars._onsetDetectionFunctionEnum, 7);

	ImGui::Checkbox("Convolve onset detection", &SPvars._convolveOnsetDetection);
	if (SPvars._convolveOnsetDetection) {
		ImGui::SliderInt("Convolve window size", &SPvars._convolveWindowSize, 1, 100);
	}

	ImGui::Text("Onset detection function:");
	imguiHistoryPlotter(SignalProcessingManager::_noteOnset->getOnsetHistory(SPvars._convolveOnsetDetection));
	ImGui::Text("Inferred peaks (passed to tempo):");
	imguiHistoryPlotter(SignalProcessingManager::_noteOnset->getDisplayPeaks());

	ImGui::SliderFloat("Peak threshold (top X%)", &SPvars._thresholdPercentForPeak, 1.0f, 25.0f);

	ImGui::End();
}

void UI::tempoDetectionUi()
{
	ImGui::Begin("Tempo", &_showTempoDetectionUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Tempo", &SPvars._computeTempoDetection);

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

	ImGui::SliderFloat("Max Tempo", &SPvars.MAX_TEMPO, 30, 250, "%.1f");
	if (ImGui::IsItemEdited() && SPvars.MAX_TEMPO < SPvars.MIN_TEMPO) {
		SPvars.MIN_TEMPO = SPvars.MAX_TEMPO;
	}
	ImGui::SliderFloat("Min Tempo", &SPvars.MIN_TEMPO, 30, 250, "%.1f");
	if (ImGui::IsItemEdited() && SPvars.MAX_TEMPO < SPvars.MIN_TEMPO) {
		SPvars.MAX_TEMPO = SPvars.MIN_TEMPO;
	}


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

	ImGui::Checkbox("Compute MFCCs", &SPvars._computeMFCCs);

	ImGui::Separator();

	static bool show0thMFCC = true;

	ImGui::PlotHistogram("Mel band energies", SignalProcessingManager::_mfccs->getBandEnergies(), SignalProcessingManager::_mfccs->getNumMelBands(), 0, 0, 0, 1, ImVec2(320,40) );
	ImGui::PlotHistogram("Mel spectrogram", SignalProcessingManager::_mfccs->getMelSpectrogram(), SignalProcessingManager::_mfccs->getNumMelBands(), 0, 0, FLOAT_MAX, FLOAT_MAX, ImVec2(320, 40));
	ImGui::PlotHistogram("MFCCs", &(SignalProcessingManager::_mfccs->getMfccs()[!show0thMFCC]), SignalProcessingManager::_mfccs->getNumMelBands() - !show0thMFCC, 0, 0, FLOAT_MAX, FLOAT_MAX, ImVec2(320, 40));

	ImGui::Checkbox("Show 1st MFCC coeff (tracks loudness)", &show0thMFCC);

	ImGui::End();
}

void UI::selfSimilarityMatrixUi()
{
	ImGui::Begin("Self Similarity Matrix", &_showSelfSimilarityMatrixUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Self Similarity Matrix", &SPvars._computeSimilarityMatrix);


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

	ImGui::BeginChild("Matrix settings", ImVec2(350, 415), true);

	//which one to calculate (future or real time)--
	static int realTimeOrFuture = 0;
	ImGui::Text("Link to:");
	ImGui::RadioButton("Real time", &realTimeOrFuture, 0); ImGui::SameLine();
	ImGui::RadioButton("Future (recommended)", &realTimeOrFuture, 1);

	if (SPvars._useFutureSimilarityMatrix != (realTimeOrFuture == 1)) {
		changedMatrixSettings = true;
	}
	//--

	//matrix size--
	static int matrixSizeTmp = SPvars._nextSimilarityMatrixSize;
	if (SPvars._wasCPSautoDecreased) {
		SPvars._nextSimilarityMatrixSize *= SPvars._CPSreduceFactor;
		matrixSizeTmp = SPvars._nextSimilarityMatrixSize;
	}
	ImGui::SliderInt("Matrix Size", &matrixSizeTmp, 1, 1000);
	std::string timeOverInfo = "covers " + std::to_string((float(matrixSizeTmp) / SPvars._desiredCPS)).substr(0, 4) + "s";
	ImGui::Text(timeOverInfo.c_str());

	if (matrixSizeTmp != SPvars._nextSimilarityMatrixSize) {
		SPvars._nextSimilarityMatrixSize = matrixSizeTmp;
		changedMatrixSettings = true;
	}
	//--

	//fast or slow texture creation--
	static int fastOrSlow = SPvars._fastSimilarityMatrixTexture;
	ImGui::Text("Texture creation type:");
	ImGui::RadioButton("Fast", &fastOrSlow, 1); ImGui::SameLine();
	ImGui::RadioButton("Slow", &fastOrSlow, 0);

	if (fastOrSlow != SPvars._fastSimilarityMatrixTexture) {
		SPvars._fastSimilarityMatrixTexture = fastOrSlow;
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
		ImGui::SliderInt("##", &low, 1, SPvars._numMelBands);
		ImGui::PopID();

		ImGui::Text("to");

		static int high = 13;
		ImGui::PushID(1);
		ImGui::SliderInt("##", &high, 1, SPvars._numMelBands);
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
	static int measureTypeEnum = SPvars._matrixMeasureEnum;
	ImGui::Text("Matrix measure type:");
	ImGui::RadioButton("Similarity", &measureTypeEnum, 0); ImGui::SameLine();
	ImGui::RadioButton("Percussion", &measureTypeEnum, 1);

	if (measureTypeEnum != SPvars._matrixMeasureEnum) {
		SPvars._matrixMeasureEnum = measureTypeEnum;
	}
	//--
	ImGui::SliderFloat("Texture contrast", &SPvars._similarityMatrixTextureContrastFactor, 1, 100, "%.3f", ImGuiSliderFlags_Logarithmic);

	if (changedMatrixSettings && ImGui::Button("Confirm", ImVec2(80, 25))) {
		SignalProcessingManager::_similarityMatrix->reInit(SPvars._nextSimilarityMatrixSize);

		SPvars._useFutureSimilarityMatrix = (realTimeOrFuture == 1); //must call before linking function to link to right thing
		linkingFunction(); //even if no change in linking function must relink after resize. Because this is static will remember last function linked to
		changedMatrixSettings = false;
	}

	if (SPvars._wasSignalProcessingReset) {
		linkingFunction();
	}

	if (SPvars._wasCPSautoDecreased) {
		SignalProcessingManager::_similarityMatrix->reInit(SPvars._nextSimilarityMatrixSize);
		linkingFunction();
	}

	ImGui::EndChild();
	//***

	imguiHistoryPlotter(SignalProcessingManager::_similarityMatrix->matrix.getSimilarityMeasureHistory());

	ImGui::End();
}

void UI::endSPui() {
	SPvars._wasSignalProcessingReset = false;
	SPvars._wasCPSautoDecreased = false;
}
//***

void UI::processFileMenuSelection()
{
	//stop common problem
	if ((_save || _saveAs) && !VisualiserManager::isVisualiserLoaded()) {
		UIglobalFeatures::queueError("Cannot save, no visualiser loaded");
		_save = false;
		_saveAs = false;
	}

	if (_new) {
		static char nameBuf[25];
		if (textInputPrompt("Name of visualiser", nameBuf, 25, _new)) {
			if (!VisualiserManager::createNewVisualiser(nameBuf)) {
				UIglobalFeatures::queueError("Failed to create new visualiser " + std::string(nameBuf));
			}
			nameBuf[0] = NULL; //reset
		}
		return;
	}

	if (_save) {
		if (!VisualiserManager::save()) {
			UIglobalFeatures::queueError("Failed to save");
		}
		_save = false;
		return;
	}

	if (_saveAs) {
		static char nameBuf[25];
		if (textInputPrompt("Save as", nameBuf, 25, _saveAs)) {
			if (!VisualiserManager::saveAsNew(nameBuf)) {
				UIglobalFeatures::queueError("Failed to save as " + std::string(nameBuf));
			}
			nameBuf[0] = NULL; //reset
		}
		return;
	}

	if (_load) {

		std::string visualiserPath = "";
		if (PFDapi::folderChooser("Chooser folder containing the .cfg of the visualiser to load",
			Vengine::IOManager::getProjectDirectory() + "/Visualisers", visualiserPath, false))
		{
			if (visualiserPath.size() < Vengine::IOManager::getProjectDirectory().size() + 1) {
				_load = false;
				return;
			}

			//will always work as cannot go outside of start path
			visualiserPath = visualiserPath.substr(Vengine::IOManager::getProjectDirectory().size() + 1); //+1 to remove '/' at start

			if (!VisualiserManager::loadVisualiser(visualiserPath)) {
				UIglobalFeatures::queueError("Failed to save, likely no visualiser loaded");
			}
		}

		_load = false;
		return;
	}
}

bool UI::textInputPrompt(const std::string& message, char* buf, int bufSize, bool& isPromptOpen)
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

	isPromptOpen = promptNotForceClosed && !confirmedName;
	return confirmedName; //return true when user either closes window or confirms name of copy
}

void UI::imguiHistoryPlotter(History<float>* history)
{
	ImGui::PlotLines("##", history->dataStartPtr(), history->totalSize(), history->firstPartOffset(), 0, 3.4028235E38F, 3.4028235E38F, ImVec2(360, 60));
}
