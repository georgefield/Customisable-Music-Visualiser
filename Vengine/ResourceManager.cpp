#include "ResourceManager.h"


#include <cmath>

using namespace Vengine;

TextureCache ResourceManager::_textureCache; //static variables must be declared in cpp file
ShaderCache ResourceManager::_shaderCache;

GLtexture ResourceManager::getTexture(std::string textureFilepath) {
	return _textureCache.getTexture(textureFilepath);
}

GLSLProgram* ResourceManager::getShaderProgram(std::string vertPath, std::string fragPath, std::string& errorOut){
	return _shaderCache.getProgram(vertPath, fragPath, errorOut);
}

GLSLProgram* Vengine::ResourceManager::reloadShaderProgram(std::string vertPath, std::string fragPath, std::string& errorOut)
{
	_shaderCache.deleteProgram(vertPath, fragPath);
	return _shaderCache.getProgram(vertPath, fragPath, errorOut);
}

void Vengine::ResourceManager::unloadAllResources()
{
	_shaderCache.unloadAll();
	_textureCache.unloadAll();
}
