#include "VisualiserManager.h"
#include "VisVars.h"
#include <Vengine/IOManager.h>

Visualiser VisualiserManager::_current;
int VisualiserManager::_timeSinceSaveTimerId;
bool VisualiserManager::_saved = false;

void VisualiserManager::init()
{
	Vengine::MyTiming::createTimer(_timeSinceSaveTimerId);
}

bool VisualiserManager::createNewVisualiser(std::string name)
{
	//check it doesnt exist already
	if ( Vengine::IOManager::directoryExists(VisVars::_userCreatedVisualiserPath + name)){
		Vengine::warning("Cannot create visualiser, visualiser with name '" + name + "' already exists.");
		return false;
	}

	//init if it does
	return _current.initNewBlankVis(VisVars::_userCreatedVisualiserPath + name);
}

bool VisualiserManager::loadVisualiser(std::string path)
{
	//check it exists
	if (!Vengine::IOManager::directoryExists(path)) {
		Vengine::warning("Cannot load visualiser with path '" + path + "' as it does not exist.");
		return false;
	}

	//init if it does
	return _current.initFromExisting(path);
}

bool VisualiserManager::save()
{
	if (!_current.isInitialised()) {
		Vengine::warning("Visualiser not initialised and cannot be saved");
		return false;
	}

	if (_current.getPath() == VisVars::_startupVisualiserPath) {
		Vengine::warning("Cannot save over startup visualiser");
		return false;
	}

	if (_current.save()) {
		Vengine::MyTiming::resetTimer(_timeSinceSaveTimerId);
		Vengine::MyTiming::startTimer(_timeSinceSaveTimerId);
		_saved = true;
		return true;
	}
	return false;
}

bool VisualiserManager::saveAsNew(std::string name)
{
	//check if visualiser with that name exists already
	if (Vengine::IOManager::directoryExists(VisVars::_userCreatedVisualiserPath + name)) {
		Vengine::warning("Visualiser with that name already exists in user created visualisers, no copy made");
		return false;
	}

	//set current visualiser to be a save of whats being worked on now in a different folder
	if (_current.initNewAsCurrentVis(VisVars::_userCreatedVisualiserPath + name)) {
		Vengine::MyTiming::resetTimer(_timeSinceSaveTimerId);
		Vengine::MyTiming::startTimer(_timeSinceSaveTimerId);
		_saved = true;
		return true;
	}
	return false;
}

bool VisualiserManager::recentlySaved(){
	if (Vengine::MyTiming::readTimer(_timeSinceSaveTimerId) < 2 && _saved)
		return true;
	
	if (_saved == true)
		_saved = false;

	return false;
}

std::string VisualiserManager::externalToInternalTexture(std::string texturePath)
{
	//copy to textures folder
	Vengine::IOManager::copyFile(texturePath, VisualiserManager::texturesFolder() + texturePath.substr(texturePath.find_last_of('/')));
	//returns name
	return texturePath.substr(texturePath.find_last_of('/') + 1);
}

std::string VisualiserManager::externalToInternalShader(std::string shaderPath)
{
	//copy to shaders folder
	Vengine::IOManager::copyFile(shaderPath, VisualiserManager::shadersFolder() + shaderPath.substr(shaderPath.find_last_of('/')));
	//returns name
	return shaderPath.substr(shaderPath.find_last_of('/') + 1);
}
