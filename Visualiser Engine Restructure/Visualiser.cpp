#include "Visualiser.h"
#include "VisualiserShaderManager.h"

bool Visualiser::initNew(const std::string& path)
{
	_path = path;

	//create visualiser directory
	if (!Vengine::IOManager::createFolder(_path, false)) {
		return false;
	}

	//create necessary directories in visualiser save
	if (!createConfig() ||
		!Vengine::IOManager::createFolder(_path + "/textures", false) ||
		!Vengine::IOManager::createFolder(_path + "/shaders", false))
	{
		return false;
	}
	//set shader path
	VisualiserShaderManager::setCurrentVisualiser(_path);

	_initialised = true;
}

bool Visualiser::initExisting(const std::string& path)
{
	_path = path;

	//read config into accessible struct if existing visualiser
	std::vector<std::string> configBuffer;
	if (!Vengine::IOManager::readTextFileToBuffer(_path + "/config.cfg", configBuffer)) {
		Vengine::warning("Could not init visualiser from folder " + _path);
		return false;
	}
	ConfigManager::textToConfig(configBuffer, _config);

	//set shader path
	VisualiserShaderManager::setCurrentVisualiser(_path);

	_initialised = true;
	return true;
}

void Visualiser::save()
{
	updateConfig();
}

void Visualiser::updateConfig()
{
	//when updating first clear old config
	Vengine::IOManager::clearTextFile(_path + "/config.cfg");
	createConfig();
}

bool Visualiser::createConfig()
{
	//create/output _config struct to config file
	std::vector<std::string> configAsText;
	ConfigManager::configToText(_config, configAsText);
	return Vengine::IOManager::outputTextFile(_path + "/config.cfg", configAsText);
}