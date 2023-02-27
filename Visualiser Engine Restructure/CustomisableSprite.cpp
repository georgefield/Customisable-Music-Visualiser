#include "CustomisableSprite.h"
#include <Vengine/MyImgui.h>

#include "MyFuncs.h"
#include "Tools.h"
#include "VisualiserManager.h"
#include "VisualiserShaderManager.h"
#include "PFDapi.h"
#include "UI.h" //for the static functions

//#include <imgui_stdlib.h> //for input text functions

CustomisableSprite::CustomisableSprite(int id, const std::string& name, Vengine::Viewport* viewport, Vengine::Window* window) :
	_viewport(viewport),
	_window(window),

	_name(name),
	_spriteState(SELECTED),
	_justCreated(true),
	_show(true),
	id(id),


	//imgui vars
	_isOptionsEnlarged(false),
	_minDistFromBLofGUItoBottomBorder(-0.2), //can go into bottom border 0.2
	_minDistFromBRofGUItoRightBorder(-1) //can go into right border 1
{
}


void CustomisableSprite::init(Vengine::Model* model, glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType)
{
	Sprite::init(model, pos, dim, depth, textureFilepath, glDrawType);
	_justCreated = true;

	_visualiserShader = VisualiserShaderManager::getShader(VisualiserShaderManager::getDefaultFragmentShaderPath());
	_texture = Vengine::ResourceManager::getTexture("Resources/NO_TEXTURE.png");

	Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames);
	Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), _shaderFileNames, true, ".frag");
}


void CustomisableSprite::draw(){

	//draw imgui
	if (_spriteState == SELECTED) {
		drawUi();
	}

	//dont draw sprite if not show
	if (!_show) {
		return;
	}

	//draw sprite
	Sprite::draw();
}

void CustomisableSprite::drawUi() {

	//-- using set next is faster
	updateOptionsRect();
	glm::vec2 optionsTLpx = Tools::openGLposToPx({ _optionsRect.x, _optionsRect.y + _optionsRect.w }, _window, _viewport);
	glm::vec2 optionsSizePx = Tools::openGLdimToPx({ _optionsRect.z, _optionsRect.w }, _window, _viewport);

	ImGui::SetNextWindowPos(ImVec2(optionsTLpx.x, optionsTLpx.y));
	ImGui::SetNextWindowSize(ImVec2(optionsSizePx.x, optionsSizePx.y));

	///*** gui for sprite ***
	ImGui::Begin(_name.c_str(), (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	//enlarge option only if no override on rect
	ImGui::Checkbox("Enlarge settings", &_isOptionsEnlarged);

	//rename--
	static char renamer[25] = "";
	ImGui::InputTextWithHint("##", "Enter text here", renamer, IM_ARRAYSIZE(renamer));
	ImGui::SameLine();
	if (ImGui::Button("Rename")) {
		if (renamer[0] != NULL) { //string is length > 0
			_name = renamer;
		}
	}
	//--

	ImGui::Separator();

	//choose shader for sprite
	shaderChooser();

	//if shader contains texture uniform then
	if (_visualiserShader->containsTextureUniform()) {
		ImGui::Separator();
		textureChooser();
	}
	else {
		ImGui::Text("No texture uniform in shader");
	}

	ImGui::Separator();

	uniformSetterUi();

	ImGui::Separator();

	glm::vec4 modelRect = getModelBoundingBox();
	//choose pos--
	//convert to pixel coords
	glm::vec2 pixelPos;
	pixelPos = Tools::openGLposToPx(glm::vec2(modelRect.x, modelRect.y), _window, _viewport, true);
	int posX = pixelPos.x;
	int posY = pixelPos.y;
	ImGui::InputInt("Pos X", &posX);
	ImGui::InputInt("Pos Y", &posY);
	//convert to opengl coords
	glm::vec2 openGLpos = Tools::pxPosToOpenGL(glm::vec2(posX, posY), _window, _viewport, true); 
	setModelPos(openGLpos);
	//--

	ImGui::Separator();

	//choose size--
	//convert to pixel coords
	glm::vec2 pixelSize;
	pixelSize = Tools::openGLdimToPx(glm::vec2(modelRect.z, modelRect.w), _window, _viewport, true);
	int sizeX = pixelSize.x;
	int sizeY = pixelSize.y;
	ImGui::InputInt("Size X", &sizeX);
	ImGui::InputInt("Size Y", &sizeY);
	//convert to opengl coords
	glm::vec2 openGLsize = Tools::pxDimToOpenGL(glm::vec2(sizeX, sizeY), _window, _viewport, true);
	setModelDim(openGLsize);
	//--

	ImGui::Separator();

	//choose depth--
	float depth = _depth;
	ImGui::InputFloat("Depth", &depth);
	setDepth(depth);
	//--

	ImGui::Separator();
	ImGui::Text("Colour");

	//choose colour--
	Vengine::ColourRGBA8 colour = getModelColour();

	//choose rbg
	static int r = colour.r;
	ImGui::SliderInt("red", &r, 0, 255);
	colour.r = GLubyte(r);

	static int g = colour.g;
	ImGui::SliderInt("green", &g, 0, 255);
	colour.g = GLubyte(g);

	static int b = colour.b;
	ImGui::SliderInt("blue", &b, 0, 255);
	colour.b = GLubyte(b);

	//choose alpha
	static int alpha = colour.a;
	ImGui::SliderInt("alpha", &alpha, 0, 255);
	colour.a = GLubyte(alpha); 

	setModelColour(colour);
	//--

	ImGui::Separator();

	//delete self--
	if (ImGui::Button("Delete")) {
		_spriteState = DELETE_SELF;
	}
	//--

	ImGui::End();
	//***
}

void CustomisableSprite::textureChooser()
{
	ImGui::Text("Set texture to pass to shader:");

	//choose whether texture or not
	static bool applyTexture = false;
	ImGui::Checkbox("Apply Texture", &applyTexture);
	if (ImGui::IsItemEdited() && !applyTexture) {
		_texture = Vengine::ResourceManager::getTexture("Resources/NO_TEXTURE.png");
	}

	if (applyTexture) {

		ImGui::SameLine();
		if (ImGui::Button("Use external texture")) {
			std::string chosenFile = "";
			if (PFDapi::fileChooser("Choose texture", Vengine::IOManager::getProjectDirectory(), chosenFile, { "PNG images (.png)", "*.png" }, true)) {
				std::cout << chosenFile << " < chosen" << std::endl;
				//copy to textures folder and then load that texture
				_texture = Vengine::ResourceManager::getTexture(VisualiserManager::externalToInternalTexture(chosenFile));
				//refresh files
				Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames, true, ".png"); //refresh
			}
		}

		//choose texture--
		ImGui::BeginChild("Texture options", ImVec2(ImGui::GetContentRegionAvail().x * 0.8, 130), true, ImGuiWindowFlags_HorizontalScrollbar);
		if (ImGui::Button("Refresh")) {
			Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames, true, ".png");
		}
		for (int i = 0; i < _textureFileNames.size(); i++) {
			if (ImGui::SmallButton(_textureFileNames[i].c_str())) { //<- explore arrow button option, might be included directory chooser?, slows down program, maybe get user to type filename (still show list)
				_texture = Vengine::ResourceManager::getTexture(VisualiserManager::texturesFolder() + "/" + _textureFileNames[i]);
			}
		}
		ImGui::EndChild();
		//--
	}

}

void CustomisableSprite::shaderChooser()
{
	ImGui::Text("Set sprite shader:");

	//select shader folder
	if (ImGui::Button("Refresh")) {
		Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), _shaderFileNames, true, ".frag");
	}
	ImGui::SameLine();
	if (ImGui::Button("Use external shader")) {
		std::string chosenFile = "";
		if (PFDapi::fileChooser("Choose shader to add", Vengine::IOManager::getProjectDirectory(), chosenFile, { "Fragment Files (.frag)", "*.frag" }, true)) {
			_visualiserShader = VisualiserShaderManager::getShader(chosenFile);
			Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), _shaderFileNames, true, ".frag"); //refresh
		}
	}


	//choose shader
	ImGui::BeginChild("Shader options", ImVec2(ImGui::GetContentRegionAvail().x * 0.8, 130), true, ImGuiWindowFlags_HorizontalScrollbar);

	for (int i = 0; i < _shaderFileNames.size(); i++) {
		if (ImGui::SmallButton(_shaderFileNames[i].c_str())) {
			_visualiserShader = VisualiserShaderManager::getShader(VisualiserManager::shadersFolder() + "/" + _shaderFileNames[i]);
		}
	}
	ImGui::EndChild();
}

void CustomisableSprite::uniformSetterUi()
{
	//*** CURRENT PAIRING INFO ***

	std::vector<std::string> intSetUniformNames;
	_visualiserShader->getSetIntUniformNames(intSetUniformNames);

	std::vector<std::string> floatSetUniformNames;
	_visualiserShader->getSetFloatUniformNames(floatSetUniformNames);

	//display integer pairings
	for (int i = 0; i < intSetUniformNames.size(); i++) {
		std::string info = _visualiserShader->getIntUniformSetterStruct(intSetUniformNames[i])->setterName + " -> " + intSetUniformNames[i];
		ImGui::Text(info.c_str()); ImGui::SameLine();
		if (ImGui::Button("Erase")) {
			_visualiserShader->unsetSetterUniformPair(intSetUniformNames[i]);
		}
	}
	//display float pairings
	for (int i = 0; i < floatSetUniformNames.size(); i++) {
		std::string info = _visualiserShader->getFloatUniformSetterStruct(floatSetUniformNames[i])->setterName + " -> " + floatSetUniformNames[i];
		ImGui::Text(info.c_str()); ImGui::SameLine();
		if (ImGui::Button("Erase")) {
			_visualiserShader->unsetSetterUniformPair(floatSetUniformNames[i]);
		}
	}

	ImGui::Separator();

	
	//*** UI FOR CREATING NEW PAIRING ***

	//	*** INTS ***

	//show possible new pairing information
	std::vector<std::string> intUnsetUniformNames; //any unset uniform
	_visualiserShader->getUnsetIntUniformNames(intUnsetUniformNames);

	//(updater functions stored in visualiser manager) (MAYBE MOVE TO VISUALISER SHADER MANAGER WOULD MAKE MORE SENSE)
	std::vector<std::string> intPossibleUniformSetterFunctionNames; //can be paired with any function
	VisualiserManager::getIntUpdaterFunctionNames(intPossibleUniformSetterFunctionNames);

	if (intSetUniformNames.size() + intUnsetUniformNames.size() > 0) {

		//choose from uniforms to set--
		std::string uniformComboStr = UI::ImGuiComboStringMaker(intUnsetUniformNames);

		const char* uniformItems = uniformComboStr.c_str();
		static int currentUniform = 0;
		ImGui::PushID(0);
		ImGui::Combo("Int setters", &currentUniform, uniformItems, intUnsetUniformNames.size());
		ImGui::PopID();
		//--

		ImGui::Text("To be set to:");

		//choose from uniforms setters for uniform--
		std::string uniformSetterComboStr = UI::ImGuiComboStringMaker(intPossibleUniformSetterFunctionNames);

		const char* uniformSetterItems = uniformSetterComboStr.c_str();
		static int currentUniformSetter = 0;
		ImGui::PushID(1);
		ImGui::Combo("Int setters", &currentUniformSetter, uniformSetterItems, intPossibleUniformSetterFunctionNames.size());
		ImGui::PopID();
		//--

		//button to confirm
		if (ImGui::Button("Confirm") && intPossibleUniformSetterFunctionNames.size() > 0 && intUnsetUniformNames.size() > 0) {
			_visualiserShader->initSetterUniformPair(intUnsetUniformNames.at(currentUniform), VisualiserManager::getIntSetterFunction(intPossibleUniformSetterFunctionNames.at(currentUniformSetter)));
		}
	}
	else {
		ImGui::Text("No integer uniforms in shader");
	}


	//	*** FLOATS ***

	//show possible new pairing information
	std::vector<std::string> floatUnsetUniformNames; //any unset uniform
	_visualiserShader->getUnsetFloatUniformNames(floatUnsetUniformNames);

	//(updater functions stored in visualiser manager) (MAYBE MOVE TO VISUALISER SHADER MANAGER WOULD MAKE MORE SENSE)
	std::vector<std::string> floatPossibleUniformSetterFunctionNames; //can be paired with any function
	VisualiserManager::getFloatUpdaterFunctionNames(floatPossibleUniformSetterFunctionNames);

	if (floatSetUniformNames.size() + floatUnsetUniformNames.size() > 0) {

		//choose from uniforms to set--
		std::string uniformComboStr = UI::ImGuiComboStringMaker(floatUnsetUniformNames);

		const char* uniformItems = uniformComboStr.c_str();
		static int currentUniform = 0;
		ImGui::PushID(2);
		ImGui::Combo("Int setters", &currentUniform, uniformItems, floatUnsetUniformNames.size());
		ImGui::PopID();
		//--

		ImGui::Text("To be set to:");

		//choose from uniforms setters for uniform--
		std::string uniformSetterComboStr = UI::ImGuiComboStringMaker(floatPossibleUniformSetterFunctionNames);

		const char* uniformSetterItems = uniformSetterComboStr.c_str();
		static int currentUniformSetter = 0;
		ImGui::PushID(3);
		ImGui::Combo("Int setters", &currentUniformSetter, uniformSetterItems, floatPossibleUniformSetterFunctionNames.size());
		ImGui::PopID();
		//--

		//button to confirm
		if (ImGui::Button("Confirm") && floatPossibleUniformSetterFunctionNames.size() > 0 && floatUnsetUniformNames.size() > 0) {
			_visualiserShader->initSetterUniformPair(floatUnsetUniformNames.at(currentUniform), VisualiserManager::getIntSetterFunction(floatPossibleUniformSetterFunctionNames.at(currentUniformSetter)));
		}
	}
	else {
		ImGui::Text("No float uniforms in shader");
	}

	//THEN FIX SPRITE BATCHING
	//THEN ADD ALL UNIFORM SETTER FUNCTIONS AS OPTIONS WHERE APPLICABLE (NOTE ONSET etc)
	//THEN THINK OF HOW TO LAY OUT CONFIG
	//THEN THINK OF HOW TO SAVE/LOAD CONFIG
	//THEN USE https://github.com/nothings/stb IMAGE LOADER INSTEAD OF THE CRAP YOURE USING NOW
	//THEN CREATE MUSIC LOADING AND QUEUEING UI
	//(so not much then)
}


void CustomisableSprite::processInput(Vengine::InputManager* inputManager){

	//opengl mouse pos
	glm::vec2 mousePos = Tools::pxPosToOpenGL(inputManager->getMouseCoords(), _window, _viewport);


	//click down
	if (!_justCreated) {
		if (inputManager->isKeyPressed(SDL_BUTTON_LEFT)) {
			if (Tools::posWithinRect(mousePos, getModelBoundingBox())) { //pos within sprite bounding box
				Vengine::MyTiming::startTimer(_timerID);
				_posOfMouseAtClick = mousePos;
				_posOfSpriteAtClick = getModelPos();
			}
			else if (Tools::posWithinRect(mousePos, _optionsRect) && _spriteState == SELECTED) {
				//do nothing
			}
			else { 
				_spriteState = NOT_SELECTED;
			}
		}

		//dragging while held down, selected
		if (_spriteState == SELECTED && inputManager->isKeyDown(SDL_BUTTON_LEFT) && Tools::posWithinRect(mousePos, getModelBoundingBox())) {

			setModelPos(_posOfSpriteAtClick + (mousePos - _posOfMouseAtClick));
		}

		if (inputManager->isKeyReleased(SDL_BUTTON_LEFT) && Tools::posWithinRect(mousePos, getModelBoundingBox())) {
			if (Vengine::MyTiming::readTimer(_timerID) < 0.3) {
				_spriteState = (_spriteState == SELECTED ? NOT_SELECTED : SELECTED); //flip state
			}
		}
	}
	else {
		_justCreated = false; //_justCreated used so it doesnt immediantly deselect
	}

}


//changes location to help stay visible when sprite moved around
void CustomisableSprite::updateOptionsRect()
{
	glm::vec2 optionsDim;
	if (_isOptionsEnlarged) {
		optionsDim = { 1.0, 1.0 };
	}
	else {
		optionsDim = { 0.8, 0.8 };
	}

	glm::vec2 optionsPos = getModelPos();
	optionsPos.y -= optionsDim.y;


	//if BL corner is closer than 200 pixels to bottom of window, move options to top right
	if (optionsPos.y - _minDistFromBLofGUItoBottomBorder < -1) {
		glm::vec2 modelDim = getModelDim();
		optionsPos.y += modelDim.y;
		optionsPos.x += modelDim.x;

		//top left if not too close to right border
		if (optionsPos.x + optionsDim.x + _minDistFromBRofGUItoRightBorder > 1) {
			optionsPos.x -= modelDim.x;
			optionsPos.x -= optionsDim.x;
		}
	}

	_optionsRect = { optionsPos.x, optionsPos.y, optionsDim.x, optionsDim.y };
}
