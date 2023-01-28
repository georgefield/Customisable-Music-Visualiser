#include "CustomisableSprite.h"
#include <Vengine/MyImgui.h>

#include "MyFuncs.h"
//#include <imgui_stdlib.h> //for input text functions

CustomisableSprite::CustomisableSprite(const std::string& name, Vengine::Viewport* viewport, Vengine::Window* window) :
	_viewport(viewport),
	_window(window),
	_name(name),
	_spriteState(SELECTED),
	_justCreated(true),

	//imgui vars
	_isOptionsEnlarged(false),
	_minPixelsBetweenGUIandBottom(120),
	_minPixelsBetweenGUIandRightSide(160)
{
}


void CustomisableSprite::init(Vengine::Model* model, glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType)
{
	Sprite::init(model, pos, dim, depth, textureFilepath, glDrawType);
	_justCreated = true;

	Vengine::IOManager::getFilesInDir("Textures/", _textureFileNames);
	Vengine::IOManager::getFilesInDir("Shaders/", _shaderFileNames, false, ".frag");
}


void CustomisableSprite::draw(){

	//draw imgui
	if (_spriteState == SELECTED) {
		drawUi();
	}



	//draw sprite
	Sprite::draw();
}

void CustomisableSprite::drawUi() {
	//-- using set next is faster
	setOptionsWindowPosAndDim();
	glm::vec2 optionsTLpx = opengGLposToPx({ _optionsRect.x, _optionsRect.y + _optionsRect.w });
	glm::vec2 optionsSizePx = opengGLdimToPx({ _optionsRect.z, _optionsRect.w });

	ImGui::SetNextWindowPos(ImVec2(optionsTLpx.x, optionsTLpx.y));
	ImGui::SetNextWindowSize(ImVec2(optionsSizePx.x, optionsSizePx.y));

	///*** gui for sprite ***
	ImGui::Begin(_name.c_str(), (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

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

	//choose texture--
	ImGui::BeginChild("Texture options", ImVec2(ImGui::GetContentRegionAvail().x * 0.8, 130), true, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::Button("Refresh")) {
		Vengine::IOManager::getFilesInDir("Textures/", _textureFileNames);
	}
	for (int i = 0; i < _textureFileNames.size(); i++) {
		if (ImGui::SmallButton(_textureFileNames[i].c_str())) { //<- explore arrow button option, might be included directory chooser?, slows down program, maybe get user to type filename (still show list)
			_texture = Vengine::ResourceManager::getTexture("Textures/" + _textureFileNames[i]);
		}
	}
	ImGui::EndChild();
	//--

	//choose shader--
	ImGui::BeginChild("Shader options", ImVec2(ImGui::GetContentRegionAvail().x * 0.8, 130), true, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::Button("Refresh")) {
		Vengine::IOManager::getFilesInDir("Shaders/", _shaderFileNames, false, ".frag");
	}
	for (int i = 0; i < _shaderFileNames.size(); i++) {
		if (ImGui::SmallButton(_shaderFileNames[i].c_str())) {
			attachShader(Vengine::ResourceManager::getShaderProgram("Shaders/" + _shaderFileNames[i]));
		}
	}
	ImGui::EndChild();
	//--

	//choose size--
	glm::vec4 modelRect = getModelBoundingBox();
	//convert to pixel coords
	glm::vec2 pixelSize;
	MyFuncs::OpenGLsizeToPixelSize(glm::vec2(modelRect.z, modelRect.w), pixelSize, _window->getScreenDim());
	int sizeX = pixelSize.x;
	int sizeY = pixelSize.y;
	ImGui::InputInt("Size X", &sizeX);
	ImGui::InputInt("Size Y", &sizeY);
	//convert to opengl coords
	glm::vec2 OpenGLsize;
	MyFuncs::PixelSizeToOpenGLsize(glm::vec2(sizeX, sizeY), OpenGLsize, _window->getScreenDim());
	setModelDim(OpenGLsize);
	//--

	//delete self--
	if (ImGui::Button("Delete")) {
		_spriteState = DELETE_SELF;
	}
	//--

	ImGui::End();
	//***
}


void CustomisableSprite::processInput(Vengine::InputManager* inputManager){

	glm::vec2 mousePos = getOpenGLmouseCoords(inputManager);


	//click down
	if (!_justCreated) {
		if (inputManager->isKeyPressed(SDL_BUTTON_LEFT)) {
			if (MyFuncs::posWithinRect(mousePos, getModelBoundingBox())) { //pos within sprite bounding box
				Vengine::MyTiming::startTimer(_timerID);
				_posOfMouseAtClick = mousePos;
				_posOfSpriteAtClick = getModelPos();
			}
			else if (MyFuncs::posWithinRect(mousePos, _optionsRect) && _spriteState == SELECTED) {
				//do nothing
			}
			else { 
				_spriteState = NOT_SELECTED;
			}
		}

		//dragging while held down, selected
		if (_spriteState == SELECTED && inputManager->isKeyDown(SDL_BUTTON_LEFT) && MyFuncs::posWithinRect(mousePos, getModelBoundingBox())) {

			setModelPos(_posOfSpriteAtClick + (mousePos - _posOfMouseAtClick));
		}

		if (inputManager->isKeyReleased(SDL_BUTTON_LEFT) && MyFuncs::posWithinRect(mousePos, getModelBoundingBox())) {
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
void CustomisableSprite::setOptionsWindowPosAndDim()
{
	glm::vec2 optionsDim;
	if (_isOptionsEnlarged)
		optionsDim = { 1.0, 1.0 };
	else
		optionsDim = { 0.8, 0.8 };

	glm::vec2 optionsPos = getModelPos();
	optionsPos.y -= optionsDim.y;

	_optionsRect = { optionsPos.x, optionsPos.y, optionsDim.x, optionsDim.y };
	//std::cout << optionsPos.x << " " << optionsPos.y << " " << optionsDim.x << " " << optionsDim.y << std::endl;

	/*//if BL corner is closer than 200 pixels to bottom of window, move options to top right
	if (pixelRect.y + _minPixelsBetweenGUIandBottom > _viewport->height) {
		_optionsPosInPixels.y -= pixelRect.w; //will be inline with top
		if (pixelRect.x + pixelRect.z + _minPixelsBetweenGUIandRightSide > _viewport->width) {
			_optionsPosInPixels.x -= _optionsDimInPixels.x;
		}
		else {
			_optionsPosInPixels.x += pixelRect.z;
		}
	}*/
}

glm::vec2 CustomisableSprite::getOpenGLmouseCoords(Vengine::InputManager* inputManager)
{
	glm::vec2 mousePos = inputManager->getMouseCoords(); //relative to viewport
	//openGLposToPx backwards
	mousePos.y -= _window->getScreenHeight() - _viewport->height;

	//get in 0->1 space
	mousePos.x /= _viewport->width;
	mousePos.y /= _viewport->height;
	//flip y
	mousePos.y = 1.0f - mousePos.y;

	mousePos.x *= 2.0f; mousePos.y *= 2.0f;
	mousePos.x -= 1.0f; mousePos.y -= 1.0f;


	return mousePos;
}

glm::vec2 CustomisableSprite::opengGLposToPx(glm::vec2 openGLpos) {

	glm::vec2 pos = openGLpos;
	//0 -> 1 screen space BL (0,0)
	pos.x += 1.0f; pos.y += 1.0f;
	pos.x *= 0.5f; pos.y *= 0.5f;
	//flip y
	pos.y = 1.0f - pos.y;
	//scale
	pos.x *= _viewport->width;
	pos.y *= _viewport->height;
	//adjust as viewport from bottom left not top left
	pos.y += _window->getScreenHeight() - _viewport->height;

	return pos;
}

glm::vec2 CustomisableSprite::opengGLdimToPx(glm::vec2 openGLdim) {

	glm::vec2 dim = openGLdim;
	//scale
	dim.x *= _viewport->width * 0.5;
	dim.y *= _viewport->height * 0.5;
	//no other adjusting than scaling needed as it is a distance

	return dim;
}
glm::vec4 CustomisableSprite::opengGLrectToPx(glm::vec4 openGLrect) {

	std::cout << _viewport->height << std::endl;

	glm::vec2 pos = opengGLposToPx({ openGLrect.x, openGLrect.y });
	glm::vec2 dim = opengGLdimToPx({ openGLrect.z, openGLrect.w });

	return glm::vec4(pos, dim);
}
