#pragma once
#include <vector>
#include <string>

class ConfigManager {
public:
	static bool outputConfigFromVisualiser(std::string configPath);
	static bool initVisualiserFromConfig(std::string configPath);
};