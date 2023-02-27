#include "ResourceManager.h"


#include <cmath>

using namespace Vengine;

TextureCache ResourceManager::_textureCache; //static variables must be declared in cpp file
ShaderCache ResourceManager::_shaderCache;

GLtexture ResourceManager::getTexture(std::string textureFilepath) {
	return _textureCache.getTexture(textureFilepath);
}

GLSLProgram* ResourceManager::getShaderProgram(std::string vertPath, std::string fragPath){
	return _shaderCache.getProgram(vertPath, fragPath);
}

void Vengine::ResourceManager::unloadAllResources()
{
	_shaderCache.unloadAll();
	_textureCache.unloadAll();
}
