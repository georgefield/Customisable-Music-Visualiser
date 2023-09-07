#include "UI.h"
#include <Vengine/MyImgui.h>
#include "VisualiserManager.h"
#include "VisualiserShaderManager.h"
#include "FourierTransformManager.h"
#include <algorithm>
#include "UIglobalFeatures.h"
#include "VisPaths.h"

#include "PFDapi.h"


UI::UI()
{}


void UI::init(Vengine::Window* window, Vengine::InputManager* inputManager) {
	UIglobalFeatures::_showUI = true;
	_showInfoBoxes = true;

	_window = window;
	_inputManager = inputManager;

	_sidebarSizePx = 200;
	_toolbarSizePx = _sidebarSizePx / _window->getAspectRatio();
}


void UI::toolbar() {

	if (!UIglobalFeatures::_showUI) {
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
		ImGui::MenuItem("Show UI (ctrl+a)", NULL, &UIglobalFeatures::_showUI);
		ImGui::MenuItem("Show Info Boxes (ctrl+x)", NULL, &_showInfoBoxes);
		ImGui::MenuItem("Fullscreen", NULL, &_fullscreen);
		ImGui::EndMenu();
	}

	processDisplayAdjustments();


	if (ImGui::BeginMenu("Shader managing")) {
		ImGui::MenuItem("Shader help", NULL, &_showShaderHelpUi);
		ImGui::MenuItem("Create shader", NULL, &_showCreateShaderUi);
		ImGui::MenuItem("Open shader folder", NULL, &_openShaderFolder);
		ImGui::MenuItem("Shader keywords", NULL, &_showShaderVariablesUi);
		ImGui::MenuItem("Show shader errors", NULL, &_showSyntaxErrorsUi);
		ImGui::EndMenu();
	}


	if (_showInfoBoxes) {
		if (_showShaderHelpUi)
			shaderHelpUi();
		if (_showShaderVariablesUi)
			shaderVariablesUi();
		if (_showCreateShaderUi)
			createShaderUi();
		if (_showSyntaxErrorsUi)
			syntaxErrorsUi();
		if (_openShaderFolder)
			openAsyncFileExplorer();
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
	ImGui::BeginChild("add button child", ImVec2(ImGui::GetContentRegionAvail().y, ImGui::GetContentRegionAvail().y / 2), false);

	if (ImGui::Button("Add sprite", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {
		ImGui::OpenPopup("add menu");
	}
	if (ImGui::BeginPopup("add menu")) {
		if (ImGui::Selectable("Quad"))
			SpriteManager::addSprite(Vengine::Mod_Quad);
		if (ImGui::Selectable("Triangle"))
			SpriteManager::addSprite(Vengine::Mod_Triangle);
		if (ImGui::Selectable("Circle"))
			SpriteManager::addSprite(Vengine::Mod_Circle);
		if (ImGui::Selectable("Ring")) {
			SpriteManager::addSprite(Vengine::Mod_Ring);
			//SpriteManager::addSprite(Vengine::Mod_Ring);
		}
		ImGui::EndPopup();
	}

	ImGui::EndChild();
	//--

	ImGui::SameLine();

	//audio playback ui--
	ImGui::BeginChild("playback child", ImVec2(280, ImGui::GetContentRegionAvail().y / 1.3), false);

	//choose whether to use loopback or not--
	static enum AudioMode { FromFile, Loopback };
	static int currentAudioMode = FromFile;
	const char* modeNames[2] = { "Audio From File", "PC Audio" };
	const char* currentModeName = (currentAudioMode >= 0 && currentAudioMode < 2) ? modeNames[currentAudioMode] : "Unknown";
	ImGui::PushID(3);
	ImGui::SliderInt("##", &currentAudioMode, 0, 1, currentModeName);
	ImGui::PopID();
	if (ImGui::IsItemEdited()) {
		AudioManager::setUseLoopback(currentAudioMode == Loopback);
		SignalProcessingManager::reset();
	}
	//--

	if (AudioManager::isUsingLoopback()) {
		//display which audio device loopback is using
		ImGui::Text("Using audio device:");
		std::string shortenedDeviceName = AudioManager::getLoopbackDeviceName();
		if (shortenedDeviceName.size() > 40)
			shortenedDeviceName = (shortenedDeviceName.substr(0, 37) + "...");
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), shortenedDeviceName.c_str());
		//display how many samples have been used from loopback so far
		ImGui::Text((std::to_string(AudioManager::_currentPlaybackInfo->sampleCounter) + " samples used").c_str());
	}
	else {
		//load song
		if (ImGui::Button("Load song")) {
			std::string chosenAudio;
			PFDapi::fileChooser("Choose audio", Vengine::IOManager::getProjectDirectory(), chosenAudio, { "Audio files", "*.wav *.mp3 *.flac" }, true);
			AudioManager::load(chosenAudio);
			SignalProcessingManager::reset();
		}

		//music name
		std::string audioInfo = "\"" + AudioManager::getAudioFileName() + "\"";
		ImGui::Text(audioInfo.c_str());

		//audio scrubbing
		ImGui::SetNextItemWidth(100);

		static int scrubber = 0;
		ImGui::PushID(4);
		ImGui::DragInt("##", &scrubber, 5000.0f, 0, AudioManager::_currentPlaybackInfo->sampleDataArrayLength, "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopID();
		if (ImGui::IsItemEdited() || ImGui::IsItemDeactivated()) {
			AudioManager::seekToSample(scrubber);
		}
		if (!ImGui::IsItemActive() && AudioManager::_currentPlaybackInfo->isAudioPlaying) {
			scrubber = AudioManager::getCurrentSample();
		}
		if (ImGui::IsItemActive()) {
			std::string scrubberSampleInfoStr = std::to_string(scrubber) + " / " + std::to_string(AudioManager::_currentPlaybackInfo->sampleDataArrayLength);
			ImGui::SetTooltip(("Go to sample " + scrubberSampleInfoStr).c_str());
		}

		ImGui::SameLine();

		//audio play/pause
		if (!AudioManager::_currentPlaybackInfo->isAudioPlaying && ImGui::ArrowButton("Play", ImGuiDir_Right)) {
			AudioManager::play();
		}
		else if (AudioManager::_currentPlaybackInfo->isAudioPlaying && ImGui::ArrowButton("Pause", ImGuiDir_Square)) {
			AudioManager::pause();
		}

		ImGui::SameLine();

		//music info
		ImGui::Text(AudioManager::getAudioTimeInfoStr(scrubber).c_str());
	}

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("db meter", ImVec2(150, ImGui::GetContentRegionAvail().y / 1.5));

	float logAmplitude = SignalProcessingManager::getMasterPtr()->getPeakAmplitudeDb() + 12;
	ImGui::PlotHistogram("0 db\n-3 db\n-6 db\n-9 db", &logAmplitude, 1, 0, "##", 0, 13, ImVec2(20, 65));
	//get correct tooltip value (take away 12)
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip((std::to_string(logAmplitude - 12) + " db").c_str());

	ImGui::SameLine();

	static float volume = 1;
	ImGui::VSliderFloat("Volume", ImVec2(30, 65), &volume, 0, 1, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	AudioManager::setVolume(volume);

	ImGui::EndChild();
	//--

	ImGui::SameLine();


	//background colour picker--
	ImGui::ColorEdit3("Background colour", Vis::vars._clearColour, ImGuiColorEditFlags_NoInputs);
	if (ImGui::IsItemEdited()) {
		glClearColor(Vis::vars._clearColour[0], Vis::vars._clearColour[1], Vis::vars._clearColour[2], 1.0f);
	}
	//--


	//visualiser name, new line--
	ImGui::Text(VisualiserManager::path().c_str());
	//--

	ImGui::SameLine();

	if (VisualiserManager::recentlySaved()) {
		ImGui::TextColored(ImVec4(0.5, 1, 0.5, 1), "Save successful");
	}

	ImGui::End();
}


void UI::sidebar() {
	if (!UIglobalFeatures::_showUI) {
		return;
	}

	ImGui::SetNextWindowPos(ImVec2(_window->getScreenWidth() - _sidebarSizePx, _toolbarSizePx));
	ImGui::SetNextWindowSize(ImVec2(_sidebarSizePx, _window->getScreenHeight() - _toolbarSizePx));

	ImGui::Begin("Sidebar", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	//change sprite values from sidebar--

	std::vector<CustomisableSprite*> spritesByDepthOrder = SpriteManager::getDepthSortedSprites();

	ImGui::Text("Sprites");
	ImGui::Text("Ctrl+d to deselect all");
	ImGui::BeginChild("Sprites", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

	bool sortRequired = false;
	for (int i = spritesByDepthOrder.size() - 1; i >= 0; i--) {
		ImGui::PushID(i);

		ImGui::Separator();

		auto it = spritesByDepthOrder[i];

		//name
		ImGui::Text(it->_spriteInfo.name);

		//change depth
		ImGui::InputFloat("Depth", &it->_depth);
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			it->updateSpriteInfoToMatchDepth();
			sortRequired = true; //sort if depth changed
		}

		//show in editor
		ImGui::Checkbox("Show (editor)", it->getShowInEditorPtr());
		if (ImGui::IsItemEdited() && *it->getShowInEditorPtr() == false) {
			it->setIfSelected(false);
		}

		//select sprite
		if (it->isSelected()) {
			if (ImGui::Button("Deselect")) {
				SpriteManager::deselectCurrent();
				it->setIfSelected(false);
			}
		}
		else if (ImGui::Button("Select")) {
			SpriteManager::deselectCurrent();
			it->setIfSelected(true);
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
	//shortcuts always availiable (even when typing)
	if (_inputManager->isKeyDown(SDLK_LCTRL) && _inputManager->isKeyPressed(SDLK_a)) { //ctrl a remove all ui
		UIglobalFeatures::_showUI = !UIglobalFeatures::_showUI;
	}
	if (_inputManager->isKeyDown(SDLK_LCTRL) && _inputManager->isKeyPressed(SDLK_x)) { //ctrl x dont show info boxes
		_showInfoBoxes = !_showInfoBoxes;
	}
	if (_inputManager->isKeyDown(SDLK_LCTRL) && _inputManager->isKeyPressed(SDLK_d)) { //ctrl d deselect current
		SpriteManager::deselectCurrent();
	}
	if (_inputManager->isKeyDown(SDLK_LCTRL) && _inputManager->isKeyPressed(SDLK_r)) { //ctrl r reset signal processing
		SignalProcessingManager::reset();
	}
	if (_inputManager->isKeyDown(SDLK_LCTRL) && _inputManager->isKeyPressed(SDLK_s)) { //ctrl s save visualiser
		_save = true;
	}

	//shortcuts availiable only when not typing
	if (!ImGui::GetIO().WantCaptureKeyboard) {

		//play/pause audio with space
		if (_inputManager->isKeyPressed(SDLK_SPACE)) {
			if (AudioManager::_currentPlaybackInfo->isAudioPlaying) {
				AudioManager::pause();
			}
			else {
				AudioManager::play();
			}
		}

		//exit fullscreen with esc
		if (_fullscreen && _inputManager->isKeyPressed(SDLK_ESCAPE)) {
			_fullscreen = false;
		}
	}
}


Vengine::Viewport UI::getViewport()
{
	if (!UIglobalFeatures::_showUI) {
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

void UI::shaderHelpUi()
{
	ImGui::Begin("Shader help", &_showShaderHelpUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Shader scripts are written in a modified GLSL, a C-like language.");
	ImGui::Text("To get started, use the 'Create shader' option from the 'Shader manager' menu.");
	ImGui::Text("Note - first you must create a new visualiser or load an existing one.");
	ImGui::Text("\n");
	ImGui::Text("Now you will need to open the shader file.");
	ImGui::Text("To find the shader, use the 'Open shader folder' option- this will bring up file explorer.");
	ImGui::Text("Right click on the shader you just created, and open with a text editor of your choice.");
	ImGui::Text("I recommend using notepad++ or vs code and installing a GLSL syntax highlighting plugin.");
	ImGui::Text("\n");
	ImGui::Text("Upon opening you will see the simplest possible shader (the same code as simple.visfrag).");
	ImGui::Text("To see how your shader acts, add a sprite and set its shader to your created shader.");
	ImGui::Text("In the shader, try multiplying vis_outputColour by vis_peakAmplitude.");
	ImGui::Text("Click the recompile button in the sprite info window (next to the shader selector).");
	ImGui::Text("Play some music!");
	ImGui::Text("Try multiplying vis_outputColour.r by vis_fragmentPosition.x");
	ImGui::Text("Try multiplying vis_outputColour.g by vis_fragmentPosition.y");
	ImGui::Text("Recompile!");
	ImGui::Text("\n");
	ImGui::Text("A full list of shader variables is available under the 'Shader keywords' options");
	ImGui::Text("Look at logEq.visfrag for an example of a more complicated shader");

	ImGui::End();
}

void UI::shaderVariablesUi()
{
	//*** SHOW CURRENT BINDINGS ***

	ImGui::Begin("Shader keywords", &_showShaderVariablesUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Keywords that you can use in a .visfrag shader");

	if (ImGui::CollapsingHeader("Input sprite info")) {
		ImGui::Text("Information about the current pixel and sprite the shader is being run on");
		ImGui::BulletText("vis_fragmentPosition");
		ImGui::BulletText("vis_fragmentUV");
		ImGui::BulletText("vis_spriteColour");
		ImGui::BulletText("vis_spriteTexture");
		ImGui::BulletText("vis_inColour");
	}

	if (ImGui::CollapsingHeader("Output pixel colour")) {
		ImGui::Text("Set this to the colour you want the pixel at vis_fragmentPosition to be");
		ImGui::BulletText("vis_outputColour");
	}

	if (ImGui::CollapsingHeader("Custom variables")) {
		ImGui::Text("Can set these through a shader, they are shared between shaders and persistent between frames.\nSet to zero on visualiser load");
		ImGui::BulletText("vis_userVar1");
		ImGui::BulletText("vis_userVar3");
		ImGui::BulletText("vis_userVar4");
	}

	if (ImGui::CollapsingHeader("Audio info (arrays)")) {
		ImGui::Text("Array size variables are availiable (look in 'values' tab)");
		std::vector<std::string> SSBOnames;
		VisualiserShaderManager::SSBOs::getAllSSBOnames(SSBOnames);
		std::sort(SSBOnames.begin(), SSBOnames.end()); //sort alphabetically

		for (auto& it : SSBOnames) {
			std::string infoStr = it + " = " + VisualiserShaderManager::SSBOs::getSSBOstatus(it);
			ImGui::BulletText(infoStr.c_str());
		}

	}

	if (ImGui::CollapsingHeader("Audio info (values)")) {
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
			ImGui::BulletText(infoStr.c_str());
		}
	}

	ImGui::End();
}

void UI::createShaderUi()
{
	if (VisualiserManager::path() == VisPaths::_startupVisualiserPath) {
		UIglobalFeatures::queueError("Cannot create shader without loading or creating new visualiser first");
		_showCreateShaderUi = false;
		return;
	}
	static char shaderName[25] = "";
	if (textInputPrompt("Shader name:", shaderName, 25, _showCreateShaderUi)) {
		if (!VisualiserShaderManager::createShader(shaderName)) {
			UIglobalFeatures::queueError("Could not create new shader file");
		}
		shaderName[0] = NULL;
	}
}

void UI::syntaxErrorsUi()
{
	ImGui::Begin("Shader errors", &_showSyntaxErrorsUi, ImGuiWindowFlags_AlwaysAutoResize);

	std::vector<std::string>* errorArr = UIglobalFeatures::getSyntaxErrorArray();
	ImGui::Text(("Error history (" + std::to_string(errorArr->size()) + " entries)").c_str());
	int count = 0;
	for (auto& it : *errorArr) {
		std::string title;
		if (it.size() > 40)
			title = (it.substr(0, 37) + "...");
		else
			title = it;

		ImGui::PushID(count);
		if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text(it.c_str());
		}
		ImGui::PopID();
		count++;
	}
	if (ImGui::Button("Clear")) {
		UIglobalFeatures::clearSyntaxErrorWindow();
	}

	ImGui::End();
}

//***


//*** SIGNAL PROCESSING UIs ***

void UI::generalSignalProcessingUi()
{
	ImGui::Begin("General", &_showGeneralSignalProcessingUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Frequency Bands", &Vis::vars._computeFrequencyBands);
	ImGui::Checkbox("Compute Note Onset", &Vis::vars._computeNoteOnset);
	ImGui::Checkbox("Compute Tempo Detection", &Vis::vars._computeTempoDetection);
	ImGui::Checkbox("Compute MFCCs", &Vis::vars._computeMFCCs);
	ImGui::Checkbox("Compute Self Similarity Matrix", &Vis::vars._computeSimilarityMatrix);

	ImGui::Separator();

	//fps & cps info
	std::string fpsInfo = "FPS: " + std::to_string(int(Vengine::MyTiming::getFPS()));
	ImGui::Text(fpsInfo.c_str());

	std::string cpsInfo = "Audio calculations per second (CPS): " + std::to_string(int(Vis::vars._desiredCPS));
	ImGui::Text(cpsInfo.c_str());
	ImGui::Text("Will auto decrease if too much lag");

	//-- cps limiting --
	static float CPS = Vis::vars._desiredCPS;
	if (Vis::comms._wasCPSautoDecreased)
		CPS = Vis::vars._desiredCPS;

	ImGui::SliderFloat("CPS", &CPS, 10, 120, "%.1f");
	if (CPS != Vis::vars._desiredCPS && ImGui::Button("Set CPS")) {
		Vis::vars._desiredCPS = CPS;
		SignalProcessingManager::reset();
	}
	//--

	ImGui::Text(("Estimated maximum CPS: " + std::to_string(1.0f / Vis::comms._calculationFrameTime)).c_str());
	
	//-- fps limiting --
	//limit fps checkbox
	static bool limitFPS = false;
	static float maxFPS = 240;
	ImGui::Checkbox("Limit FPS", &limitFPS);
	if (ImGui::IsItemEdited()) {
		if (limitFPS) Vengine::MyTiming::setFPSlimit(maxFPS);
		if (!limitFPS) Vengine::MyTiming::setFPSlimit(2400); //so large it is irrelevant
	}

	//fps limit slider (only shown when limit fps ticked)
	if (limitFPS) {
		ImGui::SliderFloat("FPS limit", &maxFPS, 60, 500, "%.1f");
		if (ImGui::IsItemEdited())
			Vengine::MyTiming::setFPSlimit(maxFPS);
	}
	//--
	
	//add ability to change fourier transform window size; use combo to choose from 1024 samples to 16192

	if (ImGui::Button("Reset signal processing (ctrl+r)")) {
		SignalProcessingManager::reset();
	}

	ImGui::Separator();

	if (AudioManager::_currentPlaybackInfo->isAudioLoaded) {
		std::string sampleRateInfoStr = "Sample rate: " + std::to_string(AudioManager::_currentPlaybackInfo->sampleRate);
		std::string currentSampleInfoStr = "Current sample: " + std::to_string(AudioManager::_currentPlaybackInfo->sampleCounter);
		if (!AudioManager::isUsingLoopback()) //add out of when not using loopback
			currentSampleInfoStr += " / " + std::to_string(AudioManager::_currentPlaybackInfo->sampleDataArrayLength);

		ImGui::Text((sampleRateInfoStr + ", " + currentSampleInfoStr).c_str());

		if (AudioManager::_currentPlaybackInfo->sampleDataArrayLength - AudioManager::_currentPlaybackInfo->sampleDataArrayStartPosition > Vis::consts._finalLoopbackStorageSize) {
			ImGui::PlotLines("data", &(AudioManager::_currentPlaybackInfo->sampleDataArrayPtr[AudioManager::_currentPlaybackInfo->sampleDataArrayStartPosition]), Vis::consts._finalLoopbackStorageSize, 0, 0, -1, 1, ImVec2(350, 40));
		}
	}
	else {
		ImGui::Text("No audio loaded");
	}

	ImGui::End();
}

void UI::fourierTransformsUi()
{
	ImGui::Begin("Frequency Bands", &_showFourierTransformUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute frequeny bands", &Vis::vars._computeFrequencyBands);

	//get id array
	std::vector<int> fourierTransformIds = FourierTransformManager::idArr();

	//*** modify existing transform ***

	//display already created ft and info about them
	for (int i = 0; i < fourierTransformIds.size(); i++) {
		int id = fourierTransformIds[i];
		std::string ftName = "Fourier Transform " + std::to_string(id);
		if (ImGui::CollapsingHeader(ftName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::PushID(i);

			auto ft = FourierTransformManager::getFourierTransform(id);

			//name of ft--
			ImGui::Text(("Fourier transform " + std::to_string(id)).c_str());
			//--

			//plot low res graph of ft--
			ImGui::PlotLines("##", FourierTransformManager::getFourierTransform(id)->getLowResOutput(), FourierTransformManager::getFourierTransform(id)->getLowResOutputSize());
			//--

			//show cutoff information--
			float cutoffLow = ft->_FTinfo.cutoffLow;
			ImGui::Text(("Cutoff low: " + std::to_string(cutoffLow)).c_str());
			ImGui::SameLine();
			float cutoffHigh = ft->_FTinfo.cutoffHigh;
			ImGui::Text(("Cutoff high: " + std::to_string(cutoffHigh)).c_str());
			float cutoffSmooth = ft->_FTinfo.cutoffSmoothFrac;
			ImGui::Text(("Cutoff smooth fraction: " + std::to_string(cutoffSmooth)).c_str());
			//--

			if (ImGui::CollapsingHeader("Smoothing options")) {
				//more options for ft--
				static std::vector<std::string> smoothOptions = { "Average of neighbours", "A.R.A. smoothing", "Time convolve" };
				static int smoothTypeIndex = 0;
				UIglobalFeatures::ImGuiBetterCombo(smoothOptions, smoothTypeIndex, 3);

				FourierTransform::FunctionType smoothType = FourierTransform::NO_FUNCTION;

				if (smoothTypeIndex == 0) {
					smoothType = FourierTransform::FREQUENCY_CONVOLVE;
					ImGui::SliderInt("Window width", &ft->_FTinfo.freqWindowSize, 2, 100, "%d", ImGuiSliderFlags_Logarithmic);
				}
				else if (smoothTypeIndex == 1) {
					smoothType = FourierTransform::SMOOTH;
					ImGui::SliderFloat("Attack (s)", &ft->_FTinfo.attack, 0.0f, 2.0f);
					ImGui::SliderFloat("Release (s)", &ft->_FTinfo.release, 0.0f, 2.0f);
					ImGui::SliderFloat("Acceleration (max/s)", &ft->_FTinfo.maxAccelerationPerSecond, 0.0f, 5.0f);
				}
				else if (smoothTypeIndex == 2) {
					smoothType = FourierTransform::TIME_CONVOLVE;
					ImGui::SliderInt("Window length", &ft->_FTinfo.timeWindowSize, 2, 7, "%d");
				}

				if (ImGui::Button("Add")) {
					ft->addSmoothEffect(smoothType);
				}

				ImGui::Text("Smoothing pipeline:");
				if (ft->_FTinfo.applyFirst != FourierTransform::NO_FUNCTION) {
					std::string idStr = smoothOptions[ft->_FTinfo.applyFirst];
					ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), (" " + idStr).c_str());
				}
				if (ft->_FTinfo.applySecond != FourierTransform::NO_FUNCTION) {
					std::string idStr = smoothOptions[ft->_FTinfo.applySecond];
					ImGui::SameLine(); ImGui::Text(" ->");
					ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), (" " + idStr).c_str());
				}
				if (ft->_FTinfo.applyThird != FourierTransform::NO_FUNCTION) {
					std::string idStr = smoothOptions[ft->_FTinfo.applyThird];
					ImGui::SameLine(); ImGui::Text(" ->");
					ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), (" " + idStr).c_str());
				}

				if (ImGui::Button("Remove last")) {
					ft->removeSmoothEffect();
				}
			}
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
	if (FourierTransformManager::availiableIdArr().size() == 0) {
		ImGui::End();
		return; //if at max then dont show creation ui
	}


	//imgui vars
	static float nextCutoffLow = 0.0f;
	static float nextCutoffHigh = SignalProcessingManager::getMasterPtr()->nyquist();
	static float nextCutoffSmoothFactor = 0.0f;
	ImGui::Text("Ctrl+Click to edit manually");
	ImGui::SliderFloat("Cutoff Hz low", &nextCutoffLow, 0.0f, SignalProcessingManager::getMasterPtr()->nyquist(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Cutoff Hz high", &nextCutoffHigh, 0.0f, SignalProcessingManager::getMasterPtr()->nyquist(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Cutoff smooth fraction", &nextCutoffSmoothFactor, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

	std::vector<int> availiableIds = FourierTransformManager::availiableIdArr();
	static int nextId = 0;
	UIglobalFeatures::ImGuiBetterCombo(availiableIds, nextId, 2);
	ImGui::SameLine();
	ImGui::Text("Frequency band id");

	if (ImGui::Button("Create new")) {
		if (nextCutoffHigh < nextCutoffLow) {
			UIglobalFeatures::queueError("Cutoff low cannot be above cutoff high");
		}
		else {
			FourierTransformManager::createFourierTransform(availiableIds.at(nextId), 7, nextCutoffLow, nextCutoffHigh, nextCutoffSmoothFactor);
		}
	}


	ImGui::End();
}

void UI::noteOnsetUi()
{
	ImGui::Begin("Note Onset", &_showNoteOnsetUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute note onset", &Vis::vars._computeNoteOnset);

	ImGui::Text("Onset detection function:");
	ImGui::RadioButton("Energy", &Vis::vars._onsetDetectionFunctionEnum, NoteOnset::ENERGY);
	ImGui::RadioButton("Derivative of log energy", &Vis::vars._onsetDetectionFunctionEnum, NoteOnset::DER_OF_LOG_ENERGY);
	ImGui::RadioButton("HFC derivative of log energy", &Vis::vars._onsetDetectionFunctionEnum, NoteOnset::HFC_DER_OF_LOG_ENERGY);
	ImGui::RadioButton("Spectral distance", &Vis::vars._onsetDetectionFunctionEnum, NoteOnset::SPECTRAL_DISTANCE);
	ImGui::RadioButton("Weighted phase deviation", &Vis::vars._onsetDetectionFunctionEnum, NoteOnset::SPECTRAL_DISTANCE_WITH_PHASE);
	ImGui::RadioButton("Combination", &Vis::vars._onsetDetectionFunctionEnum, NoteOnset::COMBINATION);

	if (Vis::vars._onsetDetectionFunctionEnum == NoteOnset::COMBINATION && ImGui::CollapsingHeader("Combination editor")) {
		bool hasBeenEdit = false;
		ImGui::SliderFloat("Energy mix amount", &Vis::vars._energyMixAmount, 0, 1);
		if (ImGui::IsItemEdited()) hasBeenEdit = true;
		ImGui::SliderFloat("Derivative of log energy mix amount", &Vis::vars._doleMixAmount, 0, 1);
		if (ImGui::IsItemEdited()) hasBeenEdit = true;
		ImGui::SliderFloat("HFC derivative of log energy mix amount", &Vis::vars._HFCdoleMixAmount, 0, 1);
		if (ImGui::IsItemEdited()) hasBeenEdit = true;
		ImGui::SliderFloat("Spectral distance mix amount", &Vis::vars._sdMixAmount, 0, 1);
		if (ImGui::IsItemEdited()) hasBeenEdit = true;
		ImGui::SliderFloat("Weighted phase deviation mix amount", &Vis::vars._wpdMixAmount, 0, 1);
		if (ImGui::IsItemEdited()) hasBeenEdit = true;
		if (hasBeenEdit) {
			Vis::vars._leftoverConstantFactor =
				(1 - Vis::vars._energyMixAmount) *
				(1 - Vis::vars._doleMixAmount) *
				(1 - Vis::vars._HFCdoleMixAmount) *
				(1 - Vis::vars._sdMixAmount) *
				(1 - Vis::vars._wpdMixAmount);
		}
	}

	ImGui::Checkbox("Convolve onset detection", &Vis::vars._convolveOnsetDetection);
	if (Vis::vars._convolveOnsetDetection) {
		ImGui::SliderInt("Convolve window size", &Vis::vars._convolveWindowSize, 1, 25);
	}

	ImGui::Text("Onset detection function:");
	imguiHistoryPlotter(SignalProcessingManager::_noteOnset->getOnsetHistory(Vis::vars._convolveOnsetDetection), 0, 1);
	ImGui::Text("Inferred peaks (passed to tempo):");
	imguiHistoryPlotter(SignalProcessingManager::_noteOnset->getDisplayPeaks(), 0, 1);

	//detection function compression--
	if (ImGui::CollapsingHeader("Compression")) {
		ImGui::SliderFloat("Gain", &Vis::vars._detectionFunctionGain, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Compression Threshold", &Vis::vars._detectionFunctionCompressionThreshold, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Compression Ratio", &Vis::vars._detectionFunctionCompressionRatio, 0.01f, 10.0f, "%.2f");
		ImGui::Checkbox("Clamp", &Vis::vars._clampBetween0and1);

		if (ImGui::Button("Reset")) {
			Vis::vars._detectionFunctionGain = 1.0f;
			Vis::vars._detectionFunctionCompressionThreshold = 1.0f;
			Vis::vars._detectionFunctionCompressionRatio = 1.0f;
			Vis::vars._clampBetween0and1 = false;
		}
	}
	//--

	ImGui::SliderFloat("Peak threshold (top X%)", &Vis::vars._thresholdPercentForPeak, 1.0f, 25.0f);

	ImGui::End();
}

void UI::tempoDetectionUi()
{
	ImGui::Begin("Tempo", &_showTempoDetectionUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Tempo", &Vis::vars._computeTempoDetection);

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

	ImGui::SliderFloat("Max Tempo", &Vis::vars.MAX_TEMPO, 30, 250, "%.1f");
	if (ImGui::IsItemEdited() && Vis::vars.MAX_TEMPO < Vis::vars.MIN_TEMPO) {
		Vis::vars.MIN_TEMPO = Vis::vars.MAX_TEMPO;
	}
	ImGui::SliderFloat("Min Tempo", &Vis::vars.MIN_TEMPO, 30, 250, "%.1f");
	if (ImGui::IsItemEdited() && Vis::vars.MAX_TEMPO < Vis::vars.MIN_TEMPO) {
		Vis::vars.MAX_TEMPO = Vis::vars.MIN_TEMPO;
	}


	static bool showDebug = false;
	ImGui::Checkbox("Agent information", &showDebug);
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

	ImGui::Checkbox("Compute MFCCs", &Vis::vars._computeMFCCs);

	ImGui::Separator();

	static bool show0thMFCC = true;

	ImGui::PlotHistogram("Mel band energies", SignalProcessingManager::_mfccs->getBandEnergies(), SignalProcessingManager::_mfccs->getNumMelBands(), 0, 0, 0, 1, ImVec2(320, 40));
	ImGui::PlotHistogram("Mel spectrogram", SignalProcessingManager::_mfccs->getMelSpectrogram(), SignalProcessingManager::_mfccs->getNumMelBands(), 0, 0, FLOAT_MAX, FLOAT_MAX, ImVec2(320, 40));
	ImGui::PlotHistogram("MFCCs", &(SignalProcessingManager::_mfccs->getMfccs()[!show0thMFCC]), SignalProcessingManager::_mfccs->getNumMelBands() - !show0thMFCC, 0, 0, FLOAT_MAX, FLOAT_MAX, ImVec2(320, 40));

	ImGui::Checkbox("Show 1st MFCC coeff (tracks loudness)", &show0thMFCC);

	ImGui::End();
}

void UI::selfSimilarityMatrixUi()
{
	ImGui::Begin("Self Similarity Matrix", &_showSelfSimilarityMatrixUi, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("Compute Self Similarity Matrix", &Vis::vars._computeSimilarityMatrix);


	if (UIglobalFeatures::_uiSMinfo != SignalProcessingManager::_similarityMatrix->_SMinfo || SignalProcessingManager::_similarityMatrix->_SMinfo._linkedTo == NONE) {
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "--Need to confirm--");
	}
	else {
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "--Confirmed, is calculating--");
	}

	//*** matrix settings***

	ImGui::BeginChild("Matrix settings", ImVec2(360, 440), true);

	//matrix size & resolution--
	ImGui::SliderInt("Matrix Size", &UIglobalFeatures::_uiSMinfo._matrixSize, 1, 500);
	if (UIglobalFeatures::_uiSMinfo._matrixSize > 300 && UIglobalFeatures::_uiSMinfo._downscale == 1) {
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Recommend using downscale");
	}
	ImGui::SliderInt("Downscale", &UIglobalFeatures::_uiSMinfo._downscale, 1, 5);
	std::string timeOverInfo = "covers " + std::to_string((float(UIglobalFeatures::_uiSMinfo._matrixSize) / Vis::vars._desiredCPS) * (float)UIglobalFeatures::_uiSMinfo._downscale).substr(0, 4) + "s";
	ImGui::Text(timeOverInfo.c_str());
	//--

	//link to options--
	ImGui::Text("Link to:");
	ImGui::RadioButton("MFCCs", (int*)&UIglobalFeatures::_uiSMinfo._linkedTo, LinkedTo::MFCC);
	ImGui::RadioButton("Mel band energies", (int*)&UIglobalFeatures::_uiSMinfo._linkedTo, LinkedTo::MelBandEnergies);
	ImGui::RadioButton("Mel spectrogram", (int*)&UIglobalFeatures::_uiSMinfo._linkedTo, LinkedTo::MelSpectrogram);
	ImGui::RadioButton("Fourier transform", (int*)&UIglobalFeatures::_uiSMinfo._linkedTo, LinkedTo::FT);
	//--

	//extra settings for certain links
	if (UIglobalFeatures::_uiSMinfo._linkedTo == 0) { // link to mfccs vv
		ImGui::Separator();

		ImGui::Text("MFCCs coefficients: ");

		ImGui::PushID(0);
		ImGui::SliderInt("##", &UIglobalFeatures::_uiSMinfo._coeffLow, 1, Vis::consts._numMelBands);
		ImGui::PopID();

		ImGui::Text("to");

		ImGui::PushID(1);
		ImGui::SliderInt("##", &UIglobalFeatures::_uiSMinfo._coeffHigh, 1, Vis::consts._numMelBands);
		ImGui::PopID();
	}
	if (UIglobalFeatures::_uiSMinfo._linkedTo == 3) {	// link to fourier transform vv
		ImGui::Text("Ctrl+Click to edit manually");
		ImGui::SliderFloat("Cutoff Hz low", &UIglobalFeatures::_uiSMinfo._cutoffLow, 0.0f, SignalProcessingManager::getMasterPtr()->nyquist(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Cutoff Hz high", &UIglobalFeatures::_uiSMinfo._cutoffHigh, 0.0f, SignalProcessingManager::getMasterPtr()->nyquist(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Cutoff smooth fraction", &UIglobalFeatures::_uiSMinfo._cutoffSmoothFactor, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	}
	//--

	if ((!(UIglobalFeatures::_uiSMinfo == SignalProcessingManager::_similarityMatrix->_SMinfo))
		&& ImGui::Button("Confirm", ImVec2(80, 25)))
	{
		SignalProcessingManager::_similarityMatrix->_SMinfo = UIglobalFeatures::_uiSMinfo;
		SignalProcessingManager::_similarityMatrix->reInit();
	}
	if (Vis::comms._wasCPSautoDecreased && false) {
		UIglobalFeatures::_uiSMinfo._matrixSize *= Vis::consts._CPSreduceFactor;
		SignalProcessingManager::_similarityMatrix->_SMinfo._matrixSize *= Vis::consts._CPSreduceFactor;

		if (UIglobalFeatures::_uiSMinfo == SignalProcessingManager::_similarityMatrix->_SMinfo) { //reinit if only thing changed was cps
			SignalProcessingManager::_similarityMatrix->reInit();
		}
	}

	// changes that dont require reinit--
	/*measure type of similarity matrix NO LONGER USED, SIMILARITY ONLY
	ImGui::Text("Matrix measure type:");
	ImGui::RadioButton("Similarity", (int*)&SignalProcessingManager::_similarityMatrix->_SMinfo._measureType, SIMILARITY); ImGui::SameLine();
	ImGui::RadioButton("Percussion", (int*)&SignalProcessingManager::_similarityMatrix->_SMinfo._measureType, PERCUSSION);
	UIglobalFeatures::_uiSMinfo._measureType = SignalProcessingManager::_similarityMatrix->_SMinfo._measureType;*/
	
	ImGui::Separator();

	//compute texture
	ImGui::Checkbox("Create texture", &Vis::vars._computeTexture);

	//contrast
	ImGui::SliderFloat("Contrast factor", &SignalProcessingManager::_similarityMatrix->_SMinfo._contrastFactor, 1, 100, "%.3f", ImGuiSliderFlags_Logarithmic);
	UIglobalFeatures::_uiSMinfo._contrastFactor = SignalProcessingManager::_similarityMatrix->_SMinfo._contrastFactor;
	// --

	ImGui::EndChild();
	//***

	ImGui::Text("Novelty value");
	imguiHistoryPlotter(SignalProcessingManager::_similarityMatrix->_SM->getSimilarityMeasureHistory());

	ImGui::End();
}

void UI::endSPui() {
	Vis::comms._wasSignalProcessingReset = false;
	Vis::comms._wasCPSautoDecreased = false;
}
//***

void UI::processFileMenuSelection()
{
	//have to save as if not loaded a visualiser
	if (_save && VisualiserManager::path() == VisPaths::_startupVisualiserPath) {
		_save = false;
		_saveAs = true;
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

		AudioManager::audioInterruptOccured(0);

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
				UIglobalFeatures::queueError("Failed to load visualiser");
			}
		}

		_load = false;
		return;
	}
}

void UI::processDisplayAdjustments()
{
	//set fullscreen--
	static bool isCurrentlyFullscreen = false;
	if (_fullscreen != isCurrentlyFullscreen) {
		_window->setWindowFullscreen(_fullscreen);
		isCurrentlyFullscreen = _fullscreen;
	}
	//--
}


void UI::openAsyncFileExplorer()
{
	_openShaderFolder = false;

	if (PFDapi::isAsyncFileExplorerAlive()) {
		UIglobalFeatures::queueError("Shader folder already open");
		return;
	}

	PFDapi::openAsyncFileExplorer("Shaders folder", Vengine::IOManager::getProjectDirectory() + "/" + VisualiserManager::shadersFolder());
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

void UI::imguiHistoryPlotter(History<float>* history, float scaleMin, float scaleMax)
{
	ImGui::PlotLines("##", history->dataStartPtr(), history->totalSize(), history->firstPartOffset(), 0, scaleMin, scaleMax, ImVec2(360, 60));
}
