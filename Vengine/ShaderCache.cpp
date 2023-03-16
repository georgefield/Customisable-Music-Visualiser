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

GLSLProgram* ShaderCache::getProgram(std::string vertPath, std::string fragPath, std::string& errorOut) {

	auto shaderLoc = findShader(vertPath, fragPath);
	//if it is already loaded
	if (shaderLoc != _loadedShaders.end()) {
		return (*shaderLoc)->program;
	}

	//if not, create program and return it
	_loadedShaders.push_back(new ShaderInfo(vertPath, fragPath));
	_loadedShaders.back()->createProgramObject();
	if (_loadedShaders.back()->isSyntaxError){
		errorOut = _loadedShaders.back()->error;
		return nullptr;
	}
	return _loadedShaders.back()->program;
}

void Vengine::ShaderCache::deleteProgram(std::string vertPath, std::string fragPath)
{
	auto shaderLoc = findShader(vertPath, fragPath);
	if (shaderLoc == _loadedShaders.end()) {
		Vengine::warning("Program " + fragPath + " not loaded");
		return;
	}

	_loadedShaders.erase(shaderLoc);
}

void ShaderCache::unloadAll() {
	_loadedShaders.clear(); //deletion of program objects handled by struct
}

//private

std::vector<ShaderInfo*>::iterator ShaderCache::findShader(std::string vertPath, std::string fragPath)
{
	for (auto it = _loadedShaders.begin(); it != _loadedShaders.end(); it++) {
		if ((*it)->vertPath == vertPath && (*it)->fragPath == fragPath) {
			return it;
		}
	}
	return _loadedShaders.end();
}
