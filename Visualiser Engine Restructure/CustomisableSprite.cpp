#include "CustomisableSprite.h"
#include <Vengine/MyImgui.h>

#include "MyFuncs.h"
//#include <imgui_stdlib.h> //for input text functions

CustomisableSprite::CustomisableSprite(const std::string& name, Vengine::Window* hostWindow) :
	_window(hostWindow),
	_name(name),
	_spriteState(SELECTED),
	_justCreated(true),

	//imgui vars
	_isOptionsEnlarged(false),
	_minPixelsBetweenGUIandBottom(120),
	_minPixelsBetweenGUIandRightSide(160)
{
}


void CustomisableSprite::init(glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType)
{
	BetterSprite::init(pos, dim, depth, textureFilepath, glDrawType);
	_optionsPosInPixels = pos;
	_justCreated = true;

	Vengine::IOManager::getFilesInDir("Textures/", _texFileNames);
}


void CustomisableSprite::draw(){

	//draw imgui
	if (_spriteState == SELECTED) {
		//-- using set next is faster
		setOptionsWindowPosAndDim();
		ImGui::SetNextWindowPos(ImVec2(_optionsPosInPixels.x, _optionsPosInPixels.y));
		ImGui::SetNextWindowSize(ImVec2(_optionsDimInPixels.x, _optionsDimInPixels.y));

		///*** gui for sprite ***
		ImGui::Begin(_name.c_str(), (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		ImGui::Checkbox("Enlarge settings", &_isOptionsEnlarged);

		//rename--
		static char renamer[25] = "";
		ImGui::InputTextWithHint("##","Enter text here", renamer, IM_ARRAYSIZE(renamer));
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
			Vengine::IOManager::getFilesInDir("Textures/", _texFileNames);
		}
		for (int i = 0; i < _texFileNames.size(); i++) {
			if (ImGui::SmallButton(_texFileNames[i].c_str())) { //<- explore arrow button option, might be included directory chooser?, slows down program, maybe get user to type filename (still show list)
				_texture = _texture = Vengine::ResourceManager::getTexture("Textures/" + _texFileNames[i]);
			}
		}
		ImGui::EndChild();
		//--

		//delete self--
		if (ImGui::Button("Delete")) {
			_spriteState = DELETE_SELF;
		}
		//--

		ImGui::End();
		//***
	}

	//draw quad
	BetterSprite::draw();
}


void CustomisableSprite::processInput(Vengine::InputManager* inputManager){

	glm::vec2 mousePos;
	MyFuncs::PixelCoordsToOpenGLcoords(inputManager->getMouseCoords(), mousePos, _window->getScreenDim());

	//click down
	if (!_justCreated) {
		if (inputManager->isKeyPressed(SDL_BUTTON_LEFT)) {
			if (posWithinSprite(mousePos)) {
				Vengine::MyTiming::startTimer(_timerID);
				_posOfMouseAtClick = mousePos;
				_posOfSpriteAtClick = _pos;
			}
			else if (posWithinSettings(mousePos) && _spriteState == SELECTED) {
				//do nothing
			}
			else { 
				_spriteState = NOT_SELECTED;
			}
		}

		//dragging while held down, selected
		if (_spriteState == SELECTED && inputManager->isKeyDown(SDL_BUTTON_LEFT) && posWithinSprite(mousePos)) {

			setRectPos(_posOfSpriteAtClick + (mousePos - _posOfMouseAtClick));
		}

		if (inputManager->isKeyReleased(SDL_BUTTON_LEFT) && posWithinSprite(mousePos)) {
			if (Vengine::MyTiming::readTimer(_timerID) < 0.3) {
				_spriteState = (_spriteState == SELECTED ? NOT_SELECTED : SELECTED); //flip state
			}
		}
	}
	else {
		_justCreated = false; //_justCreated used so it doesnt immediantly deselect
	}
}

bool CustomisableSprite::posWithinSettings(glm::vec2 pos)
{
	//get open pos & dim
	glm::vec2 openGLpos;
	MyFuncs::PixelCoordsToOpenGLcoords(_optionsPosInPixels, openGLpos);
	glm::vec2 openGLdim;
	MyFuncs::PixelSizeToOpenGLsize(_optionsDimInPixels, openGLdim);

	//from TL in pix, need from BL
	openGLpos.y -= openGLdim.y;

	return MyFuncs::posWithinRect(pos, glm::vec4(openGLpos, openGLdim));
}


//changes location to help stay visible when sprite moved around
void CustomisableSprite::setOptionsWindowPosAndDim()
{
	if (_isOptionsEnlarged)
		_optionsDimInPixels = { _window->getScreenWidth() / 4.0f + 150, _window->getScreenHeight() / 3.0f + 150 };
	else
		_optionsDimInPixels = { 300, 200 };


	//imgui window pos is from top left corner, sprite pos and dim is from bottom left corner
	_optionsPosInPixels = _posPix;
	//if BL corner is closer than 200 pixels to bottom of window, move options to top right
	if (_posPix.y + _minPixelsBetweenGUIandBottom > _window->getScreenHeight()) {
		_optionsPosInPixels.y -= _dimPix.y; //will be inline with top
		if (_posPix.x + _dimPix.x + _minPixelsBetweenGUIandRightSide > _window->getScreenWidth()) {
			_optionsPosInPixels.x -= _optionsDimInPixels.x;
		}
		else {
			_optionsPosInPixels.x += _dimPix.x;
		}
	}
}
