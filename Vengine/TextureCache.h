#pragma once
#include <map>
#include "GLtexture.h"
#include <string>

namespace Vengine {

	class TextureCache
	{
	public:
		TextureCache();
		~TextureCache();

		GLtexture getTexture(std::string textureFilepath);
		void unloadAll();

	private:
		std::map<std::string, GLtexture> _textureMap;
	};

}