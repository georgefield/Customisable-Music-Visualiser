#include "Visualiser.h"
#include "VisualiserShaderManager.h"

bool Visualiser::initNewBlankVis(const std::string& path)
{
	SpriteManager::reset();

	_path = path;
	VisualiserShaderManager::setCurrentVisualiser(_path);

	//create visualiser directory
	if (!Vengine::IOManager::createFolder(_path, false)) {
		return false;
	}

	Vengine::IOManager::copyDirectory("Resources/Blank Visualiser", _path);

	_initialised = true;
}

bool Visualiser::initNewAsCurrentVis(const std::string& path)
{
	//create visualiser directory
	if (!Vengine::IOManager::createFolder(path, false)) {
		return false;
	}
	Vengine::IOManager::copyDirectory(_path, path); //copy directory of what user working with now to new dir

	_path = path;
	VisualiserShaderManager::setCurrentVisualiser(_path);

	//save config of current
	if (!ConfigManager::outputConfigFromVisualiser(_path + "/config.cfg"))
		return false;

	_initialised = true;
	return true;
}

bool Visualiser::initFromExisting(const std::string& path)
{
	_path = path;

	//set shader path
	VisualiserShaderManager::setCurrentVisualiser(_path);

	if (!ConfigManager::initVisualiserFromConfig(_path + "/config.cfg"))
		return false;

	_initialised = true;
	return true;
}

bool Visualiser::save()
{
	assert(_initialised);
	return ConfigManager::outputConfigFromVisualiser(_path + "/config.cfg");
}