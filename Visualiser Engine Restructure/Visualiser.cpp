#include "Visualiser.h"

void Visualiser::initNew(const std::string& name, const std::string& fullPath)
{
	//set initial config vars and create config file if new
	_config.name = name;
	_config.fullPath = fullPath;

	createConfig();

	//create necessary directories in project save
	Vengine::IOManager::createFolder(_config.fullPath + "/textures", false);
	Vengine::IOManager::createFolder(_config.fullPath + "/shaders", false);
	Vengine::IOManager::createFolder(_config.fullPath + "/.shaders", true); //this is a hidden folder where .frags from shaders are augmented and paired with .vert for compilation

	//set shader path
	VisualiserShaderManager::setCurrentVisualiserPath(_config.fullPath);

	_initialised = true;
}

void Visualiser::initExisting(const std::string& fullPath)
{
	//read config into accessible struct if existing visualiser
	std::vector<std::string> configBuffer;
	Vengine::IOManager::readTextFileToBuffer(fullPath + "/config.cfg", configBuffer);
	ConfigManager::textToConfig(configBuffer, _config);

	//set shader path
	VisualiserShaderManager::setCurrentVisualiserPath(_config.fullPath);

	_initialised = true;
}

void Visualiser::save()
{
	updateConfig();
}

void Visualiser::updateConfig()
{
	//when updating first clear old config
	Vengine::IOManager::clearTextFile(_config.fullPath + "/config.cfg");
	createConfig();
}

void Visualiser::createConfig()
{
	//create/output _config struct to config file
	std::vector<std::string> configAsText;
	ConfigManager::configToText(_config, configAsText);
	Vengine::IOManager::outputTextFile(_config.fullPath + "/config.cfg", configAsText);
}