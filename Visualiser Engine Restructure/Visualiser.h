#pragma once
#include "SpriteManager.h"
#include "ConfigManager.h"
#include <string>
#include <vector>


class Visualiser
{
public:
	Visualiser():
		_initialised(false){}

	bool initNew(const std::string& path);
	bool initExisting(const std::string& path);

	void save();

	bool isInitialised() {
		return _initialised;
	}

	VisualiserConfig _config;

	std::string getPath() { return _path; }
	std::string getName() { int substrStart = _path.find_last_of("/"); return _path.substr(substrStart); }
private:
	std::string _path;

	bool _initialised;

	void updateConfig();
	bool createConfig();
};

