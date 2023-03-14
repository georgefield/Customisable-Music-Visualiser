#include "Visualiser.h"
#include "VisualiserShaderManager.h"

bool Visualiser::initNew(const std::string& path)
{
	_path = path;

	//create visualiser directory
	if (!Vengine::IOManager::createFolder(_path, false)) {
		return false;
	}

	if (Vengine::IOManager::directoryExists(path)) {
		Vengine::warning("Visualiser " + path + " already exists");
		return false;
	}

	//create necessary directories in visualiser save
	if (!Vengine::IOManager::createFolder(_path + "/textures", false) ||
		!Vengine::IOManager::createFolder(_path + "/shaders", false) ||
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
	if (!Vengine::IOManager::readTextFileToVector(_path + "/config.cfg", configBuffer)) {
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

bool Visualiser::updateConfig()
{
	//create/output _config struct to config file
	std::vector<std::string> configAsText;
	ConfigManager::configToText(_config, configAsText);
	std::string output = "";
	for (auto& it : configAsText) {
		output += it + "\n";
	}
	output.resize(output.size() - 1); //remove last \n
	return Vengine::IOManager::outputToTextFile(_path + "/config.cfg", output, true);
}