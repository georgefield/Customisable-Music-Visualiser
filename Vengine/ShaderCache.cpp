#include "ShaderCache.h"
#include "IOManager.h"
#include "MyErrors.h"

using namespace Vengine;


ShaderCache::ShaderCache()
{
}


ShaderCache::~ShaderCache()
{
}

GLSLProgram* ShaderCache::getProgram(std::string shaderFilepath) {

	std::string ext = shaderFilepath.substr(shaderFilepath.size() - 5, 5); //if ends in .frag or .vert will get just that
	if (ext == ".vert" || ext == ".frag") {
		warning("shader filepath ended with " + ext + ", assumed there exists both a .vert and .frag file with that name");
		shaderFilepath = shaderFilepath.substr(0, shaderFilepath.size() - 5); //truncate
	}

	auto mit = _shaderMap.find(shaderFilepath); //std::map<std::string, GLSLProgram>::iterator
	if (mit == _shaderMap.end()) { //texture not loaded yet
		_shaderMap[shaderFilepath] = new GLSLProgram();

		_shaderMap[shaderFilepath]->compileShaders(shaderFilepath + ".vert", shaderFilepath + ".frag");
		//hard coded at the moment
		_shaderMap[shaderFilepath]->addAttrib("vertexPosition");
		_shaderMap[shaderFilepath]->addAttrib("vertexColour");
		_shaderMap[shaderFilepath]->addAttrib("vertexUV");

		_shaderMap[shaderFilepath]->linkShaders();

		return _shaderMap[shaderFilepath];
	}

	return mit->second; //if it is created use map
}