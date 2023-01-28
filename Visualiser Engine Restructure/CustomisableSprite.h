#pragma once
#include <string>
#include <glm/glm.hpp>

#include <Vengine/Vengine.h>

enum SpriteState {
	NOT_SELECTED,
	SELECTED,
	DELETE_SELF
};

class CustomisableSprite : public Vengine::Sprite
{
public:
	CustomisableSprite(const std::string& name, Vengine::Viewport* viewport, Vengine::Window* window);


	void init(Vengine::Model* model, glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW);
	void draw();

	void processInput(Vengine::InputManager* inputManager);

	//getters
	Vengine::GLSLProgram* getShaderProgram() const { return _shaderProgram; };

	SpriteState getSpriteState() { return _spriteState; }
private:
	void drawUi();

	void setOptionsWindowPosAndDim();

	glm::vec2 getOpenGLmouseCoords(Vengine::InputManager* inputManager);

	glm::vec2 opengGLposToPx(glm::vec2 openGLpos);
	glm::vec2 opengGLdimToPx(glm::vec2 openGLdim);
	glm::vec4 opengGLrectToPx(glm::vec4 openGLrect);

	//important information describing sprite--
	Vengine::Viewport* _viewport;
	Vengine::Window* _window;
	std::string _name;	
	SpriteState _spriteState;
	//--

	//imgui vars--
	glm::vec4 _optionsRect; //in opengl coords

	bool _isOptionsEnlarged;
	int _minPixelsBetweenGUIandBottom;
	int _minPixelsBetweenGUIandRightSide;

	std::vector<std::string> _textureFileNames;
	std::vector<std::string> _shaderFileNames;
	//--

	//selected/dragging vars--
	glm::vec2 _posOfMouseAtClick;
	glm::vec2 _posOfSpriteAtClick;
	int _timerID;
	bool _justCreated;
	//--
};

