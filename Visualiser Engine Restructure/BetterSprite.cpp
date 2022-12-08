#include "BetterSprite.h"

#include "MyFuncs.h"

void BetterSprite::init(glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType) {

	Sprite::init(pos, dim, depth, textureFilepath, glDrawType);
	MyFuncs::OpenGLcoordsToPixelCoords(pos, _posPix);
	MyFuncs::OpenGLsizeToPixelSize(dim, _dimPix);
	_shaderProgram = Vengine::ResourceManager::getShaderProgram("Shaders/noShading");
	printf("%i", _shaderProgram->getID());
}

void BetterSprite::attachShader(Vengine::GLSLProgram* shaderProgram) {
	_shaderProgram = shaderProgram;
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

bool BetterSprite::posWithinSprite(glm::vec2 pos)
{
	return MyFuncs::posWithinRect(pos, glm::vec4(_pos, _dim));
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

void BetterSprite::updatePixInfo()
{
	MyFuncs::OpenGLcoordsToPixelCoords(_pos, _posPix);
	MyFuncs::OpenGLsizeToPixelSize(_dim, _dimPix);
}

void BetterSprite::updateBuffer(TransformType transformType){

	glBindBuffer(GL_ARRAY_BUFFER, _vboID); //bind _vboID to gl array buffer (can only have one array buffer active at one time)
	if (true) {
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(_vertexData), &_vertexData);
		if (glGetError() != GL_NO_ERROR) {
			GLenum errCode = glGetError();
			Vengine::warning("Error updating data about quad", true);
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

