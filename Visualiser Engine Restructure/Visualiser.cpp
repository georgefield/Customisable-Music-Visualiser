#include "Visualiser.h"

void Visualiser::initNew(const std::string& path)
{
	_path = path;
	createConfig();

	//create necessary directories in project save
	Vengine::IOManager::createFolder(_path + "/textures", false);
	Vengine::IOManager::createFolder(_path + "/shaders", false);
	Vengine::IOManager::createFolder(_path + "/.shaders", true); //this is a hidden folder where .frags from shaders are augmented and paired with .vert for compilation

	//set shader path
	VisualiserShaderManager::setCurrentVisualiser(_path);

	_initialised = true;
}

void Visualiser::initExisting(const std::string& path)
{
	_path = path;

	//read config into accessible struct if existing visualiser
	std::vector<std::string> configBuffer;
	if (!Vengine::IOManager::readTextFileToBuffer(_path + "/config.cfg", configBuffer)) {
		Vengine::warning("Could not init visualiser from folder " + _path);
		return;
	}
	ConfigManager::textToConfig(configBuffer, _config);

	//set shader path
	VisualiserShaderManager::setCurrentVisualiser(_path);

	_initialised = true;
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

void Visualiser::createConfig()
{
	//create/output _config struct to config file
	std::vector<std::string> configAsText;
	ConfigManager::configToText(_config, configAsText);
	Vengine::IOManager::outputTextFile(_path + "/config.cfg", configAsText);
}