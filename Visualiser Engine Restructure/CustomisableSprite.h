#pragma once
#include <string>
#include <glm/glm.hpp>
#include "VisualiserShader.h"
#include <Vengine/Vengine.h>




class CustomisableSprite : public Vengine::Sprite
{
public:
	CustomisableSprite(int id, Vengine::Viewport* viewport, Vengine::Window* window);
	const int id; //for sprite manager

	struct SpriteInfo {
		Vengine::ModelType model = Vengine::ModelType::Mod_Quad;
		float pos[2] = {-0.5, -0.5};
		float dim[2] = { 1, 1 };
		Vengine::ColourRGBA8 colour = { 255, 255, 255, 255 };
		float depth = 0.0f;
		bool applyTexture = false;
		bool useSimilarityMatrixTexture = false;
		char textureFilename[100] = { NULL };
		char shaderFilename[100] = { NULL };
		char name[25] = "unnamed sprite";
	};
	SpriteInfo _spriteInfo;

	void init(SpriteInfo info);
	void draw();

	void processInput(Vengine::InputManager* inputManager);

	//simple setters
	void updateSpriteInfoToMatchDepth() { _spriteInfo.depth = _depth; }

	void setIfSelected(bool isSelected) { _selected = isSelected; _showUi |= isSelected; _uiOpened = true; }
	void setDeleted() { _deleted = true; }

	//getters
	VisualiserShader* getVisualiserShader() const { return _visualiserShader; };

	//ui var getters--
	bool isSelected() { return _selected; }
 	bool isDeleted() { return _deleted; }
	bool isShowInEditor() { return _showInEditor; }

	bool* getShowInEditorPtr() { return &_showInEditor; }
	//--

private:

	VisualiserShader* _visualiserShader;

	void drawUi();
	void updateShader();
	void updateTexture();

	//called in draw ui
	void textureChooser();
	void shaderChooser();

	void updateOptionsRect();

	//enviroment vars--
	Vengine::Viewport* _viewport;
	Vengine::Window* _window;
	//--

	//imgui vars--
	bool _justCreated;
	bool _selected;
	bool _deleted;
	bool _showInEditor;
	bool _resetTextureCombo;
	bool _resetShaderCombo;
	bool _showUi;
	bool _uiOpened;

	glm::vec4 _optionsRect; //in opengl coords

	bool _isOptionsEnlarged;
	float _minDistFromBLofGUItoBottomBorder;
	float _minDistFromBRofGUItoRightBorder;

	std::vector<std::string> _textureFileNames;
	std::vector<std::string> _shaderFileNames;
	//--

	//selected/dragging vars--
	glm::vec2 _posOfMouseAtClick;
	glm::vec2 _posOfSpriteAtClick;
	int _timerID;
	//--

	glm::vec2 getPos() { return glm::vec2(_spriteInfo.pos[0], _spriteInfo.pos[1]); }
	glm::vec2 getDim() { return glm::vec2(_spriteInfo.dim[0], _spriteInfo.dim[1]); }

};

