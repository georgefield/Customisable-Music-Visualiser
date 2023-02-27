#include "VisualiserManager.h"
#include <Vengine/IOManager.h>

const std::string USER_CREATED_VISUALISER_PATH = "Visualisers/User Created/";
const std::string PRESET_VISUALISER_PATH = "Visualisers/Preset/";

Visualiser VisualiserManager::_current;

void VisualiserManager::init()
{
	loadVisualiser("Resources/Startup Visualiser");
}

bool VisualiserManager::createNewVisualiser(std::string name)
{
	//check it doesnt exist already
	if ( Vengine::IOManager::directoryExists(USER_CREATED_VISUALISER_PATH + name)){
		Vengine::warning("Cannot create visualiser, visualiser with name '" + name + "' already exists.");
		return false;
	}
	//init if it does
	return _current.initNew(USER_CREATED_VISUALISER_PATH + name);
}

bool VisualiserManager::loadVisualiser(std::string path)
{
	//check it exists
	if (!Vengine::IOManager::directoryExists(path)) {
		Vengine::warning("Cannot load visualiser with path '" + path + "' as it does not exist.");
		return false;
	}
	//init if it does
	return _current.initExisting(path);
}

bool VisualiserManager::save()
{
	if (!_current.isInitialised()) {
		Vengine::warning("Visualiser not initialised and cannot be saved");
		return false;
	}

	_current.save();
	return true;
}

bool VisualiserManager::saveAsNew(std::string name)
{
	if (!_current.isInitialised()) {
		Vengine::warning("Visualiser not initialised and cannot be saved");
		return false;
	}

	//check if visualiser with that name exists already
	if (Vengine::IOManager::directoryExists(USER_CREATED_VISUALISER_PATH + name)) {
		Vengine::warning("Visualiser with that name already exists in user created visualisers, no copy made");
		return false;
	}

	//create copy
	if (!Vengine::IOManager::copyDirectory(_current.getPath(), USER_CREATED_VISUALISER_PATH + name)) {
		Vengine::warning("Failed to copy files to new visualiser directory");
		return false;
	}
	
	//set current visualiser to be the copy
	_current = Visualiser();
	_current.initExisting(USER_CREATED_VISUALISER_PATH + name);

	return true;
}

std::string VisualiserManager::externalToInternalTexture(std::string texturePath)
{
	//copy to textures folder
	Vengine::IOManager::copyFile(texturePath, VisualiserManager::texturesFolder() + texturePath.substr(texturePath.find_last_of('/')));
	//return new texture
	return VisualiserManager::texturesFolder() + texturePath.substr(texturePath.find_last_of('/'));
}