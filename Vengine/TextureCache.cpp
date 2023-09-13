#include "TextureCache.h"
#include "IOManager.h"
#include "MyErrors.h"

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

		//check file exists
		if (!IOManager::fileExists(textureFilepath)) { 
			fatalError("Texture file " + textureFilepath + "does not exist");
			return {}; //0,0,0
		}

		//load texture and insert texture to map
		GLtexture newTexture = IOManager::loadImage(textureFilepath);
		_textureMap.insert(make_pair(textureFilepath, newTexture));
		return newTexture;
	}

	return mit->second; //if it is loaded use map
}

void Vengine::TextureCache::unloadAll()
{
	_textureMap.clear();
}
