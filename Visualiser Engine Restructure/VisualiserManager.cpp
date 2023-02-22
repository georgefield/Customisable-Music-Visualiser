#include "VisualiserManager.h"
#include <Vengine/IOManager.h>

const std::string USER_CREATED_VISUALISER_PATH = "Visualisers/User Created/";
const std::string PRESET_VISUALISER_PATH = "Visualisers/Preset/";

Visualiser VisualiserManager::_current;

bool VisualiserManager::createNewVisualiser(std::string name)
{
	//check it doesnt exist already
	if ( Vengine::IOManager::directoryExists(USER_CREATED_VISUALISER_PATH + name)){
		Vengine::warning("Cannot create visualiser, visualiser with name '" + name + "' already exists.");
		return false;
	}
	//init if it does
	_current.initNew(name, USER_CREATED_VISUALISER_PATH + name);

	return true;
}

bool VisualiserManager::loadVisualiser(std::string path)
{
	//check it exists
	if (!Vengine::IOManager::directoryExists(path)) {
		Vengine::warning("Cannot load visualiser with path '" + path + "' as it does not exist.");
		return false;
	}
	//init if it does
	_current.initExisting(path);

	return true;
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
	Vengine::IOManager::copyDirectory(_current._config.fullPath, USER_CREATED_VISUALISER_PATH + name);

	//set current visualiser to be the copy
	_current = Visualiser();
	_current.initExisting(USER_CREATED_VISUALISER_PATH + name);

	return true;
}
