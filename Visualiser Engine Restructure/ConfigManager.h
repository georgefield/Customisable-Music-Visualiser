#pragma once
#include <vector>
#include <string>

class ConfigManager {
public:
	static bool outputConfigFromVisualiser(std::string configPath);
	static void inputVisualiserFromConfig(std::vector<std::string>& in);
private:
};