#include "CustomisableSprite.h"
#include <Vengine/MyImgui.h>

#include "Tools.h"
#include "VisualiserManager.h"
#include "VisualiserShaderManager.h"
#include "PFDapi.h"
#include "VisVars.h"
#include "SignalProcessingManager.h"
#include "UIglobalFeatures.h"
#include "SpriteManager.h"

//#include <imgui_stdlib.h> //for input text functions

CustomisableSprite::CustomisableSprite(int id, Vengine::Viewport* viewport, Vengine::Window* window) :
	_viewport(viewport),
	_window(window),

	_selected(true),
	_deleted(false),
	_justCreated(true),
	_showInEditor(true),
	_resetTextureCombo(true),
	_resetShaderCombo(true),
	_showUi(true),
	_uiOpened(true),
	id(id),

	//imgui vars
	_isOptionsEnlarged(false),
	_minDistFromBLofGUItoBottomBorder(-0.2), //can go into bottom border 0.2
	_minDistFromBRofGUItoRightBorder(-1) //can go into right border 1
{
}

void CustomisableSprite::init(SpriteInfo spriteInfo) {
	//set up sprite--
	memcpy(&_spriteInfo, &spriteInfo, sizeof(SpriteInfo));
	Sprite::init(_spriteInfo.model, getPos(), getDim(), _spriteInfo.depth);
	_justCreated = true;

	if (_spriteInfo.shaderFilename[0] == NULL)
		strcpy_s(_spriteInfo.shaderFilename, sizeof(SpriteInfo::shaderFilename), VisVars::_defaultFragShaderPath.substr(VisVars::_defaultFragShaderPath.find_last_of("/") + 1).c_str());

	if (_spriteInfo.textureFilename[0] == NULL)
		assert(_spriteInfo.useSimilarityMatrixTexture || !_spriteInfo.applyTexture);

	setModelColour(_spriteInfo.colour);
	//apply texture and shader

	updateTexture();
	updateShader();

	//--

	//set up ui vars--
	Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames);
	Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), _shaderFileNames, true, VisVars::_visShaderExtension);

	Vengine::MyTiming::createTimer(_timerID); //timer for selection ui
	//--
}

void CustomisableSprite::draw() {

	//draw imgui
	if (_selected && _showUi) {
		drawUi();
	}

	if (_spriteInfo.useSimilarityMatrixTexture) {
		_texture = SignalProcessingManager::_similarityMatrix->matrix.getMatrixTexture();
	}

	//draw sprite
	Sprite::draw();

	_uiOpened = false;
}

void CustomisableSprite::drawUi() {

	//-- using set next is faster
	updateOptionsRect();
	glm::vec2 optionsTLpx = Tools::openGLposToPx({ _optionsRect.x, _optionsRect.y + _optionsRect.w }, _window, _viewport);
	glm::vec2 optionsSizePx = Tools::openGLdimToPx({ _optionsRect.z, _optionsRect.w }, _window, _viewport);

	ImGui::SetNextWindowPos(ImVec2(optionsTLpx.x, optionsTLpx.y));
	ImGui::SetNextWindowSize(ImVec2(optionsSizePx.x, optionsSizePx.y));

	///*** gui for sprite ***
	ImGui::Begin(_spriteInfo.name, (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	//enlarge option only if no override on rect
	ImGui::Checkbox("Enlarge settings", &_isOptionsEnlarged);

	//rename--
	static char renamer[sizeof(SpriteInfo::name)] = "";
	if (_uiOpened) {
		strcpy_s(renamer, sizeof(SpriteInfo::name), _spriteInfo.name);
	}
	ImGui::InputTextWithHint("##", "Enter text here", renamer, IM_ARRAYSIZE(renamer));
	ImGui::SameLine();
	if (ImGui::Button("Rename")) {
		if (renamer[0] != NULL) { //string is length > 0
			strcpy_s(_spriteInfo.name, sizeof(SpriteInfo::name), renamer);
		}
	}
	//--

	ImGui::Separator();

	//choose shader for sprite
	if (ImGui::CollapsingHeader("Sprite shader")) {
		shaderChooser();
		updateShader();
	}
	//if shader contains texture uniform then
	if (ImGui::CollapsingHeader("Sprite texture")){
		textureChooser();
		updateTexture();
	}

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
		_spriteInfo.pos[0] = openGLpos.x;
		_spriteInfo.pos[1] = openGLpos.y;
		setModelPos(getPos());
	}

	//slider
	ImGui::PushID(0);
	ImGui::SliderInt2("##", pos, 0, _window->getScreenWidth());
	ImGui::PopID();
	if (ImGui::IsItemEdited()) {
		glm::vec2 openGLpos = Tools::pxPosToOpenGL(glm::vec2(pos[0], pos[1]), _window, _viewport, true);
		_spriteInfo.pos[0] = openGLpos.x;
		_spriteInfo.pos[1] = openGLpos.y;
		setModelPos(getPos());
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
		_spriteInfo.dim[0] = openGLsize.x;
		_spriteInfo.dim[1] = openGLsize.y;
		setModelDim(getDim());
	}
	
	//slider
	ImGui::PushID(1);
	ImGui::SliderInt2("##", dim, 0, _window->getScreenWidth());
	if (ImGui::IsItemEdited()) {
		glm::vec2 openGLsize = Tools::pxDimToOpenGL(glm::vec2(dim[0], dim[1]), _window, _viewport, true);
		_spriteInfo.dim[0] = openGLsize.x;
		_spriteInfo.dim[1] = openGLsize.y;
		setModelDim(getDim());
	}
	ImGui::PopID();
	//--

	ImGui::Separator();

	//choose depth--
	ImGui::InputFloat("Depth", &_depth);
	if (ImGui::IsItemEdited()) {
		updateSpriteInfoToMatchDepth();
		SpriteManager::updateDepthSortedSprites();
	}
	//--

	ImGui::Separator();
	ImGui::Text("Colour");

	//choose colour--
	static float pickedColour[4];
	if (_uiOpened) {
		float currentColour[4] = { static_cast<float>(_spriteInfo.colour.r) / 255.0f, static_cast<float>(_spriteInfo.colour.g) / 255.0f, static_cast<float>(_spriteInfo.colour.b) / 255.0f, static_cast<float>(_spriteInfo.colour.a) / 255.0f };
		memcpy(pickedColour, currentColour, sizeof(currentColour));
	}
	ImGui::ColorEdit4("Sprite colour", pickedColour);
	if (ImGui::IsItemEdited()) {
		_spriteInfo.colour.r = static_cast<GLubyte>(pickedColour[0] * 255.0f);
		_spriteInfo.colour.g = static_cast<GLubyte>(pickedColour[1] * 255.0f);
		_spriteInfo.colour.b = static_cast<GLubyte>(pickedColour[2] * 255.0f);
		_spriteInfo.colour.a = static_cast<GLubyte>(pickedColour[3] * 255.0f);

		setModelColour(_spriteInfo.colour);
	}
	//--

	//choose rbg

	//--

	ImGui::Separator();

	//delete self--
	if (ImGui::Button("Delete")) {
		setDeleted();
	}
	//--

	ImGui::End();
	//***
}

void CustomisableSprite::updateShader()
{
	assert(_spriteInfo.shaderFilename[0] != NULL);
	//always need simple.visfrag
	if (!Vengine::IOManager::fileExists(VisualiserManager::shadersFolder() + "/simple.visfrag")) {
		Vengine::warning("No simple.visfrag in new visualiser: had to copy from resources");
		Vengine::IOManager::copyFile(VisVars::_defaultFragShaderPath, VisualiserManager::shadersFolder() + "/simple.visfrag");
	}

	auto tmp = VisualiserShaderManager::getShader(VisualiserManager::shadersFolder() + "/" + _spriteInfo.shaderFilename);
	if (tmp != nullptr) {
		_visualiserShader = tmp;
	}
	else {
		_resetShaderCombo;
		strcpy_s(_spriteInfo.shaderFilename, sizeof("simple.visfrag"), "simple.visfrag");
		_visualiserShader = VisualiserShaderManager::getShader(VisualiserManager::shadersFolder() + "/" + _spriteInfo.shaderFilename);
	}
}

void CustomisableSprite::updateTexture()
{
	if (!_spriteInfo.applyTexture) {
		_texture = Vengine::ResourceManager::getTexture(VisVars::_1x1WhiteTexturePath);
		return;
	}
	if (_spriteInfo.useSimilarityMatrixTexture) {
		_texture = SignalProcessingManager::_similarityMatrix->matrix.getMatrixTexture();
		return;
	}
	if (_spriteInfo.textureFilename[0] != NULL) {
		_texture = Vengine::ResourceManager::getTexture(VisualiserManager::texturesFolder() + "/" + _spriteInfo.textureFilename);
	}
}

void CustomisableSprite::textureChooser()
{
	ImGui::Text("Set texture to pass to shader:");

	//choose whether texture or not
	ImGui::Checkbox("Apply Texture", &_spriteInfo.applyTexture);
	if (!_spriteInfo.applyTexture) {
		_spriteInfo.useSimilarityMatrixTexture = false;
		return;
	}

	//refresh texture list
	if (ImGui::Button("Refresh")) {
		Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames, true, ".png");
	}

	ImGui::SameLine();

	//add texture from outside project folder
	if (ImGui::Button("Add another texture")) {
		std::string chosenFile = "";
		if (PFDapi::fileChooser("Choose texture", Vengine::IOManager::getProjectDirectory(), chosenFile, { "PNG images (.png)", "*.png" }, true) && chosenFile != "") {

			//copy to textures folder and then set that texture
			strcpy_s(_spriteInfo.textureFilename, sizeof(SpriteInfo::textureFilename), VisualiserManager::externalToInternalTexture(chosenFile).c_str());
			_spriteInfo.applyTexture = true;
			_resetTextureCombo = true;

			//refresh files
			Vengine::IOManager::getFilesInDir(VisualiserManager::texturesFolder(), _textureFileNames, true, ".png"); //refresh
		}
	}

	//choose between file or similarity matrix texture
	static int textureType = 0;
	ImGui::RadioButton("Texture file", &textureType, 0);
	ImGui::RadioButton("Similarity matrix texture", &textureType, 1);

	_spriteInfo.useSimilarityMatrixTexture = (textureType == 1);
	if (_spriteInfo.useSimilarityMatrixTexture) {
		return;
	}

	static int textureIndex = -1;
	//get correct start texture index for combo
	if (_resetTextureCombo || _uiOpened) {
		for (int i = 0; i < _textureFileNames.size(); i++) {
			if (_textureFileNames[i] == _spriteInfo.textureFilename) {
				textureIndex = i;
				_resetTextureCombo = false;
				break;
			}
		}
	}

	//choose texture from file in texture project folder
	if (UIglobalFeatures::ImGuiBetterCombo(_textureFileNames, textureIndex, 0)) {
		if (_textureFileNames.size() == 0) {
			return;
		}

		if (_textureFileNames[textureIndex].length() > sizeof(SpriteInfo::textureFilename)) {
			UIglobalFeatures::queueError("Texture filename too long " + std::to_string(_textureFileNames[textureIndex].length()) + "/" + std::to_string(sizeof(SpriteInfo::textureFilename)));
			return;
		}

		strcpy_s(_spriteInfo.textureFilename, sizeof(SpriteInfo::textureFilename), _textureFileNames[textureIndex].c_str());
		_spriteInfo.applyTexture = true;
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

	//add shader from outside project folder
	if (ImGui::Button("Add another shader")) {
		std::string chosenFile = "";
		if (PFDapi::fileChooser("Choose shader to add", Vengine::IOManager::getProjectDirectory(), chosenFile, { "Visualiser frag ( " + VisVars::_visShaderExtension + ")", " * " + VisVars::_visShaderExtension }, true)) {

			//copy to shaders folder and then set that texture
			strcpy_s(_spriteInfo.shaderFilename, sizeof(SpriteInfo::shaderFilename), VisualiserManager::externalToInternalShader(chosenFile).c_str());
			_resetShaderCombo = true;

			Vengine::IOManager::getFilesInDir(VisualiserManager::shadersFolder(), _shaderFileNames, true, VisVars::_visShaderExtension); //refresh
		}
	}

	assert(_shaderFileNames.size() >= 0);
	static int shaderIndex = -1;
	//get correct start shader index for combo
	if (_resetShaderCombo || _uiOpened) {
		for (int i = 0; i < _shaderFileNames.size(); i++) {
			if (_shaderFileNames[i] == _spriteInfo.shaderFilename) {
				shaderIndex = i;
				_resetShaderCombo = false;
				break;
			}
		}
	}

	//choose shader from shader project folder
	if (UIglobalFeatures::ImGuiBetterCombo(_shaderFileNames, shaderIndex, 1)) {
		
		if (_shaderFileNames[shaderIndex].length() > sizeof(SpriteInfo::shaderFilename)) {
			UIglobalFeatures::queueError("Shader filename too long " + std::to_string(_shaderFileNames[shaderIndex].length()) + "/" + std::to_string(sizeof(SpriteInfo::shaderFilename)));
			return;
		}

		strcpy_s(_spriteInfo.shaderFilename, sizeof(SpriteInfo::shaderFilename), _shaderFileNames[shaderIndex].c_str());
	}

	ImGui::SameLine();
	if (ImGui::Button("Recompile")) {
		VisualiserShaderManager::recompileShader(VisualiserManager::shadersFolder() + "/" + _shaderFileNames[shaderIndex]);
	}
}


void CustomisableSprite::processInput(Vengine::InputManager* inputManager) {

	//opengl mouse pos
	glm::vec2 mousePos = Tools::pxPosToOpenGL(inputManager->getMouseCoords(), _window, _viewport);

	if (inputManager->isKeyPressed(SDLK_DELETE)) {
		setDeleted();
	}

	//click down
	if (!_justCreated) {
		if (inputManager->isKeyPressed(SDL_BUTTON_LEFT)) {
			if (Tools::posWithinRect(mousePos, getModelBoundingBox())) { //pos within sprite bounding box
				Vengine::MyTiming::resetTimer(_timerID);
				Vengine::MyTiming::startTimer(_timerID);
				_posOfMouseAtClick = mousePos;
				_posOfSpriteAtClick = getModelPos();
				_showUi = false;
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
			_spriteInfo.pos[0] = (_posOfSpriteAtClick + (mousePos - _posOfMouseAtClick)).x;
			_spriteInfo.pos[1] = (_posOfSpriteAtClick + (mousePos - _posOfMouseAtClick)).y;
			setModelPos(getPos());
		}

		if (inputManager->isKeyReleased(SDL_BUTTON_LEFT) && Tools::posWithinRect(mousePos, getModelBoundingBox())) {
			_showUi = true;
			if (Vengine::MyTiming::readTimer(_timerID) < 0.2) {
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
		optionsDim = { 1.2, 1.2 };
	}
	else {
		optionsDim = { 0.8, 0.8 };
	}

	glm::vec2 optionsPos = glm::vec2(-0.95, 1.0f - optionsDim.y);
	_optionsRect = { optionsPos.x, optionsPos.y, optionsDim.x, optionsDim.y };
	return;

	//glm::vec2 optionsPos = getModelPos();
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
