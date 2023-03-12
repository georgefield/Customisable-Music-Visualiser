#pragma once
#include <string>
#include <glm/glm.hpp>
#include "VisualiserShader.h"
#include <Vengine/Vengine.h>


class CustomisableSprite : public Vengine::Sprite
{
public:
	CustomisableSprite(int id, const std::string& name, Vengine::Viewport* viewport, Vengine::Window* window);
	const int id; //for sprite manager

	void init(Vengine::Model* model, glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW);
	void draw();

	void processInput(Vengine::InputManager* inputManager);

	//simple setters
	void setDepth(float depth) { _depth = depth; }

	void setIfSelected(bool isSelected) { _selected = isSelected; }
	void setIfDeleted() { _deleted = true; }

	//getters
	VisualiserShader* getVisualiserShader() const { return _visualiserShader; };

	bool isSelected() { return _selected; }
	bool isDeleted() { return _deleted; }
	bool isShowInEditor() { return _showInEditor; }

	bool* getShowInEditorPtr() { return &_showInEditor; }
	float* getDepthPtr() { return &_depth; }

	std::string getName() const { return _name; }


private:

	VisualiserShader* _visualiserShader;

	void drawUi();

	//called in draw ui
	void textureChooser();
	void shaderChooser();

	void updateOptionsRect();

	//enviroment vars--
	Vengine::Viewport* _viewport;
	Vengine::Window* _window;

	//important information describing sprite--
	std::string _name;	
	bool _justCreated;
	bool _selected;
	bool _deleted;
	bool _showInEditor;

	//imgui vars--
	glm::vec4 _optionsRect; //in opengl coords

	bool _isOptionsEnlarged;
	float _minDistFromBLofGUItoBottomBorder;
	float _minDistFromBRofGUItoRightBorder;

	std::vector<std::string> _textureFileNames;
	std::vector<std::string> _shaderFileNames;

	bool _useSimilarityMatrixTexture;
	//--

	//selected/dragging vars--
	glm::vec2 _posOfMouseAtClick;
	glm::vec2 _posOfSpriteAtClick;
	int _timerID;
	//--
};

