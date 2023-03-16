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

	bool initNewBlankVis(const std::string& path);
	bool initNewAsCurrentVis(const std::string& path);
	bool initFromExisting(const std::string& path);

	bool save();

	bool isInitialised() {
		return _initialised;
	}

	std::string getPath() { return _path; }
	std::string getName() { int substrStart = _path.find_last_of("/"); return _path.substr(substrStart); }
private:
	std::string _path;

	bool _initialised;
};

