#pragma once
#include <GL/glew.h>
#include <SDL/SDL.h>
#include <Vengine/Vengine.h>
#include <glm/glm.hpp>

class BetterSprite : public Vengine::Sprite
{
public:

	virtual void init(glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW) override;
	void attachShader(Vengine::GLSLProgram* shaderProgram);

	//change sprite
	void setRect(glm::vec2 pos, glm::vec2 dim) { Sprite::setRect(pos, dim); updateBuffer(QuadTranslate); updatePixInfo(); }
	void setRectPos(glm::vec2 pos) { Sprite::setRect(pos, _dim); updateBuffer(QuadTranslate); updatePixInfo(); }
	void setRectCentre(glm::vec2 pos) { Sprite::setRect(glm::vec2(pos.x - (_dim.x / 2.0f), pos.y - (_dim.y / 2.0f)), _dim); updateBuffer(QuadTranslate); updatePixInfo(); }
	void setRectDim(glm::vec2 dim) { Sprite::setRect(_pos, dim); updateBuffer(QuadTranslate); updatePixInfo(); }

	void move(float up, float right);
	void rotate(float clockwiseRadians);
	void transform(glm::mat2x2 matrix);


	//test sprite
	bool posWithinSprite(glm::vec2 pos);

	//getters
	Vengine::GLSLProgram* getShaderProgram() const { return _shaderProgram; }
	float getDepth() const { return _depth; }
	Vengine::GLtexture getTexture() const { return _texture; }
	Vengine::Vertex getVertex(int i);
	glm::vec4 getQuadRect() const { return glm::vec4(_pos, _dim); }
	//hard coded as systems not all working together yet
	glm::vec4 getUvRect() const { return glm::vec4(0, 0, 1, 1); }
	Vengine::ColourRGBA8 getColour() const { return Vengine::ColourRGBA8(0, 0, 0, 1); }
protected:
	glm::vec2 _posPix;
	glm::vec2 _dimPix;
	Vengine::GLSLProgram* _shaderProgram; //non functional in this inbetween class, but needed to make sprite batch work

private:

	enum TransformType {
		QuadTranslate,
		TexTranslate
	};

	void updatePixInfo();
	void updateBuffer(TransformType transformType);
};

