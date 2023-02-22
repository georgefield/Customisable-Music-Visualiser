#pragma once
#include "VisualiserShaderManager.h"
#include "SpriteManager.h"
#include "ConfigManager.h"
#include <string>
#include <vector>


class Visualiser
{
public:
	Visualiser():
		_initialised(false){}

	void initNew(const std::string& name, const std::string& fullPath);
	void initExisting(const std::string& fullPath);

	void save();

	bool isInitialised() {
		return _initialised;
	}

	VisualiserConfig _config;

private:
	bool _initialised;

	void updateConfig();
	void createConfig();
};

