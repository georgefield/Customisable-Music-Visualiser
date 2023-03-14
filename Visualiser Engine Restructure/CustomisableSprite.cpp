#include "CustomisableSprite.h"
#include <Vengine/MyImgui.h>

#include "Tools.h"
#include "VisualiserManager.h"
#include "VisualiserShaderManager.h"
#include "PFDapi.h"
#include "VisVars.h"
#include "SignalProcessingManager.h"
#include "UIglobalFeatures.h"

//#include <imgui_stdlib.h> //for input text functions

CustomisableSprite::CustomisableSprite(int id, const std::string& name, Vengine::Viewport* viewport, Vengine::Window* window) :
	_viewport(viewport),
	_window(window),

	_name(name),
	_selected(true),
	_deleted(false),
	_justCreated(true),
	_showInEditor(true),
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

	_visualiserShader = VisualiserShaderManager::getShader(VisVars::_defaultFragShaderPath);
	_texture = Vengine::ResourceManager::getTexture("Resources/NO_TEXTURE.png");

	Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames);
	Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), _shaderFileNames, true, VisVars::_visShaderExtension);

	Vengine::MyTiming::createTimer(_timerID); //timer for selection ui

	_colour = getModelColour();
	setModelColour(_colour); //for some reason its not set straight away
}


void CustomisableSprite::draw(){

	//draw imgui
	if (_selected) {
		drawUi();
	}

	if (_useSimilarityMatrixTexture) {
		_texture = SignalProcessingManager::_similarityMatrix->matrix.getMatrixTexture();
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
	if (ImGui::CollapsingHeader("Sprite shader"))
		shaderChooser();

	//if shader contains texture uniform then
	if (ImGui::CollapsingHeader("Sprite texture"))
		textureChooser();

	ImGui::Separator();

	glm::vec4 modelRect = getModelBoundingBox();
	//choose pos--
	//convert to pixel coords
	glm::vec2 pixelPos;
	pixelPos = Tools::openGLposToPx(glm::vec2(modelRect.x, modelRect.y), _window, _viewport, true);
	static int pos[2];
	pos[0] = pixelPos.x;
	pos[1] = pixelPos.y;
	ImGui::InputInt2("Pos", pos);
	if (!ImGui::IsItemFocused()) {
		pos[0] = pixelPos.x;
		pos[1] = pixelPos.y;
	}
	//convert to opengl coords
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		glm::vec2 openGLpos = Tools::pxPosToOpenGL(glm::vec2(pos[0], pos[1]), _window, _viewport, true);
		setModelPos(openGLpos);
	}
	//--

	ImGui::Separator();

	//choose size--
	//convert to pixel coords
	glm::vec2 pixelSize;
	pixelSize = Tools::openGLdimToPx(glm::vec2(modelRect.z, modelRect.w), _window, _viewport, true);
	static int dim[2];
	dim[0] = pixelSize.x;
	dim[1] = pixelSize.y;
	ImGui::InputInt2("Size", dim);
	if (!ImGui::IsItemFocused()) {
		dim[0] = pixelSize.x;
		dim[1] = pixelSize.y;
	}
	//convert to opengl coords
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		glm::vec2 openGLsize = Tools::pxDimToOpenGL(glm::vec2(dim[0], dim[1]), _window, _viewport, true);
		setModelDim(openGLsize);
	}
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
	static float pickedColour[4] = { static_cast<float>(_colour.r) / 255.0f, static_cast<float>(_colour.g) / 255.0f,static_cast<float>(_colour.b) / 255.0f, static_cast<float>(_colour.a) / 255.0f };
	ImGui::ColorEdit4("Background colour", pickedColour);
	if (ImGui::IsItemEdited()) { 
		_colour.r = static_cast<GLubyte>(pickedColour[0] * 255.0f);
		_colour.g = static_cast<GLubyte>(pickedColour[1] * 255.0f);
		_colour.b = static_cast<GLubyte>(pickedColour[2] * 255.0f);
		_colour.a = static_cast<GLubyte>(pickedColour[3] * 255.0f);

		setModelColour(_colour);
	}
	//--

	//choose rbg

	//--

	ImGui::Separator();

	//delete self--
	if (ImGui::Button("Delete")) {
		setIfDeleted();
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

		if (ImGui::Button("Refresh")) {
			Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames, true, ".png");
		}

		ImGui::SameLine();

		if (ImGui::Button("Add another texture")) {
			std::string chosenFile = "";
			if (PFDapi::fileChooser("Choose texture", Vengine::IOManager::getProjectDirectory(), chosenFile, { "PNG images (.png)", "*.png" }, true)) {
				std::cout << chosenFile << " < chosen" << std::endl;
				//copy to textures folder and then load that texture
				_texture = Vengine::ResourceManager::getTexture(VisualiserManager::externalToInternalTexture(chosenFile));
				//refresh files
				Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames, true, ".png"); //refresh
			}
		}

		//choose between file or similarity matrix texture
		static int textureType = 0;
		ImGui::RadioButton("Texture file", &textureType, 0);
		ImGui::RadioButton("Similarity matrix texture", &textureType, 1);

		if (textureType == 0) {
			static int textureIndex = 0;

			if (UIglobalFeatures::ImGuiBetterCombo(_textureFileNames, textureIndex, 0)) {
				_texture = Vengine::ResourceManager::getTexture(VisualiserManager::texturesFolder() + "/" + _textureFileNames[textureIndex]);
			}
		}

		if (ImGui::IsItemEdited()) {
			_useSimilarityMatrixTexture = (textureType == 1);
		}
		//--
	}

}

void CustomisableSprite::shaderChooser()
{
	ImGui::Text("Set sprite shader:");

	//select shader folder
	if (ImGui::Button("Refresh")) {
		Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), _shaderFileNames, true, VisVars::_visShaderExtension);
	}

	ImGui::SameLine();

	if (ImGui::Button("Add another shader")) {
		std::string chosenFile = "";
		if (PFDapi::fileChooser("Choose shader to add", Vengine::IOManager::getProjectDirectory(), chosenFile, { "Visualiser frag ( "+ VisVars::_visShaderExtension + ")", " * " + VisVars::_visShaderExtension }, true)) {
			_visualiserShader = VisualiserShaderManager::getShader(chosenFile);
			Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), _shaderFileNames, true, VisVars::_visShaderExtension); //refresh
		}
	}

	//choose shader

	static int shaderIndex = -1;
	if (shaderIndex == -1) {
		for (int i = 0; i < _shaderFileNames.size(); i++) {
			if (_shaderFileNames[i] == VisVars::_defaultFragShaderPath.substr(VisVars::_defaultFragShaderPath.find_last_of("/") + 1)) {
				shaderIndex = i;
				break;
			}
		}
	}

	if (UIglobalFeatures::ImGuiBetterCombo(_shaderFileNames, shaderIndex, 1)) {
		_visualiserShader = VisualiserShaderManager::getShader(VisualiserManager::shadersFolder() + "/" + _shaderFileNames[shaderIndex]);
	}
}


void CustomisableSprite::processInput(Vengine::InputManager* inputManager){

	//opengl mouse pos
	glm::vec2 mousePos = Tools::pxPosToOpenGL(inputManager->getMouseCoords(), _window, _viewport);


	//click down
	if (!_justCreated) {
		if (inputManager->isKeyPressed(SDL_BUTTON_LEFT)) {
			if (Tools::posWithinRect(mousePos, getModelBoundingBox())) { //pos within sprite bounding box
				Vengine::MyTiming::resetTimer(_timerID);
				Vengine::MyTiming::startTimer(_timerID);
				_posOfMouseAtClick = mousePos;
				_posOfSpriteAtClick = getModelPos();
			}
			else if (Tools::posWithinRect(mousePos, _optionsRect) && _selected) {
				//do nothing if pos within settings rect
			}
			else { 
				_selected = false;
			}
		}

		//dragging while held down, selected
		if (_selected && inputManager->isKeyDown(SDL_BUTTON_LEFT) && Tools::posWithinRect(mousePos, getModelBoundingBox())) {

			setModelPos(_posOfSpriteAtClick + (mousePos - _posOfMouseAtClick));
		}

		if (inputManager->isKeyReleased(SDL_BUTTON_LEFT) && Tools::posWithinRect(mousePos, getModelBoundingBox())) {
			if (Vengine::MyTiming::readTimer(_timerID) < 0.3) {
				_selected = !_selected;
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
