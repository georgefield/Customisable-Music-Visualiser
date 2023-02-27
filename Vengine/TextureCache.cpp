#include "TextureCache.h"
#include "IOManager.h"

using namespace Vengine;


TextureCache::TextureCache()
{
}


TextureCache::~TextureCache()
{
}

GLtexture TextureCache::getTexture(std::string textureFilepath) {
	
	auto mit = _textureMap.find(textureFilepath); //std::map<std::string, GLtexture>::iterator
	if (mit == _textureMap.end()) { //texture not loaded yet
		GLtexture newTexture = IOManager::loadPNG(textureFilepath);
		_textureMap.insert(make_pair(textureFilepath, newTexture));
		return newTexture;
	}

	return mit->second; //if it is loaded use map
}

void Vengine::TextureCache::unloadAll()
{
	_textureMap.clear();
}
