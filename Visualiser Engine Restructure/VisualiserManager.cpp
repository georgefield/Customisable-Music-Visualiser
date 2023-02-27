#include "VisualiserManager.h"
#include <Vengine/IOManager.h>

const std::string USER_CREATED_VISUALISER_PATH = "Visualisers/User Created/";
const std::string PRESET_VISUALISER_PATH = "Visualisers/Preset/";

Visualiser VisualiserManager::_current;

std::unordered_map<std::string, UniformSetter<float>> VisualiserManager::_floatFunctions;
std::unordered_map<std::string, UniformSetter<int>> VisualiserManager::_intFunctions;

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

void VisualiserManager::addPossibleUniformSetter(std::string name, std::function<float()> function)
{
	if (_floatFunctions.find(name) != _floatFunctions.end()) {
		Vengine::warning("Function with name '" + name + "' already in uniform updater list. Uniform updater list not changed.");
		return;
	}

	_floatFunctions[name].initialiseAsDynamic(name, function);
}

void VisualiserManager::addPossibleUniformSetter(std::string name, std::function<int()> function)
{
	if (_intFunctions.find(name) != _intFunctions.end()) {
		Vengine::warning("Function with name '" + name + "' already in uniform updater list. Uniform updater list not changed.");
		return;
	}

	_intFunctions[name].initialiseAsDynamic(name, function);
}

void VisualiserManager::addPossibleUniformSetter(std::string name, float constant)
{
	if (_floatFunctions.find(name) != _floatFunctions.end()) {
		Vengine::warning("Function with name '" + name + "' already in uniform updater list. Uniform updater list not changed.");
		return;
	}
	
	_floatFunctions[name].initialiseAsConstant(name, constant);

}

void VisualiserManager::addPossibleUniformSetter(std::string name, int constant)
{
	if (_intFunctions.find(name) != _intFunctions.end()) {
		Vengine::warning("Function with name '" + name + "' already in uniform updater list. Uniform updater list not changed.");
		return;
	}

	_intFunctions[name].initialiseAsConstant(name, constant);

}

void VisualiserManager::deletePossibleUniformSetter(std::string name)
{
	if (_floatFunctions.find(name) != _floatFunctions.end()) {
		_floatFunctions.erase(name);
		return;
	}

	if (_intFunctions.find(name) != _intFunctions.end()) {
		_intFunctions.erase(name);
		return;
	}

	Vengine::warning("Function '" + name + "' not in the uniform updater function map");
	return;
}

void VisualiserManager::getFloatUpdaterFunctionNames(std::vector<std::string>& names)
{
	for (auto& it : _floatFunctions) {
		names.push_back(it.first);
	}
}

void VisualiserManager::getIntUpdaterFunctionNames(std::vector<std::string>& names)
{
	for (auto& it : _intFunctions) {
		names.push_back(it.first);
	}
}

UniformSetter<float> VisualiserManager::getFloatSetterFunction(std::string name)
{
	if (_floatFunctions.find(name) == _floatFunctions.end()) {
		Vengine::warning("No setter function with name " + name);
		UniformSetter<float> ret;
		ret.initialiseAsNotSet();
		return ret;
	}

	return _floatFunctions[name];
}

UniformSetter<int> VisualiserManager::getIntSetterFunction(std::string name)
{
	if (_intFunctions.find(name) == _intFunctions.end()) {
		Vengine::warning("No setter function with name " + name);
		UniformSetter<int> ret;
		ret.initialiseAsNotSet();
		return ret;
	}

	return _intFunctions[name];
}

