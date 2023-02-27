#pragma once
#include "GLSLProgram.h"
#include "TextureCache.h"
#include "ShaderCache.h"

#include <map>
#include <vector>
#include <GL/glew.h>

namespace Vengine {

	class ResourceManager
	{
	public:
		static GLtexture getTexture(std::string textureFilepath); //loads texture in aswell
		static GLSLProgram* getShaderProgram(std::string vertPath, std::string fragPath); //loads shader in aswell
		static void unloadAllResources();

	private:
		static TextureCache _textureCache;
		static ShaderCache _shaderCache;

	};

}