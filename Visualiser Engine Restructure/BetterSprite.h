#pragma once
#include <GL/glew.h>
#include <SDL/SDL.h>
#include <Vengine/Vengine.h>
#include <glm/glm.hpp>

class BetterSprite : public Vengine::Sprite
{
public:

	void init(glm::vec2 pos, glm::vec2 dim, float depth = 0.0f, std::string textureFilepath = "", GLuint glDrawType = GL_DYNAMIC_DRAW);

	//change sprite
	void setRect(glm::vec2 pos, glm::vec2 dim) { Sprite::setRect(pos, dim); updateBuffer(QuadTranslate); }
	void setRectPos(glm::vec2 pos) { Sprite::setRect(pos, _dim); updateBuffer(QuadTranslate); }
	void setRectCentre(glm::vec2 pos) { Sprite::setRect(glm::vec2(pos.x - (_dim.x / 2.0f), pos.y - (_dim.y / 2.0f)), _dim); updateBuffer(QuadTranslate); }
	void setRectDim(glm::vec2 dim) { Sprite::setRect(_pos, dim); updateBuffer(QuadTranslate); }

	void move(float up, float right);
	void rotate(float clockwiseRadians);
	void transform(glm::mat2x2 matrix);



	//test sprite
	bool posWithinSprite(glm::vec2 xy);
protected:
	float _depth;
private:

	enum TransformType {
		QuadTranslate,
		TexTranslate
	};

	void updateBuffer(TransformType transformType);
};

