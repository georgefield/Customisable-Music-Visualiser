#include "BetterSprite.h"



void BetterSprite::init(glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType){

	_depth = depth;
	Sprite::init(pos, dim, textureFilepath, glDrawType);
}


void BetterSprite::move(float up, float right){

	for (int i = 0; i < 6; i++) {
		_vertexData[i].position.y += up;
		_vertexData[i].position.x += right;
	}
	updateBuffer(QuadTranslate);
}

void BetterSprite::rotate(float clockwiseRadians){
	Vengine::fatalError("not implemented rotate yet");
}

void BetterSprite::transform(glm::mat2x2 matrix){

	for (int i = 0; i < 6; i++) {
		glm::vec2 vertexVec(_vertexData[i].position.x, _vertexData[i].position.y);
		glm::vec2 result = vertexVec * matrix;
		_vertexData[i].position.x = result.x;
		_vertexData[i].position.y = result.y;
	}
	updateBuffer(QuadTranslate);
}

bool BetterSprite::posWithinSprite(glm::vec2 xy)
{
	//vertex data [2] & [3] are on diagonal, [2] BL, [3] TR
	if (xy.x >= _vertexData[2].position.x && xy.x <= _vertexData[3].position.x
		&& xy.y >= _vertexData[2].position.y && xy.y <= _vertexData[3].position.y) {
		return true;
	}
	return false;
}

Vengine::Vertex BetterSprite::getVertex(int i)
{
	if (i >= 0 && i < 6) {
		return _vertexData[i];
	}
	else {
		Vengine::fatalError("vertex index out of range (i > 5 or i < 0)");
		return Vengine::Vertex();
	}
}

//--private

void BetterSprite::updateBuffer(TransformType transformType){

	glBindBuffer(GL_ARRAY_BUFFER, _vboID); //bind _vboID to gl array buffer (can only have one array buffer active at one time)
	if (true) {
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(_vertexData), &_vertexData);
		if (glGetError() != GL_NO_ERROR) {
			printf("error");
		}
	}
	else { //dont use this for now
		if (transformType == QuadTranslate) {
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(_vertexData->position), &_vertexData); //change only vertex position data
		}
		else if (transformType == TexTranslate) {
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(_vertexData->position), sizeof(_vertexData->uv), &_vertexData);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind _vboID
}

