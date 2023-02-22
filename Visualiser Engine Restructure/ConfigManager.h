#pragma once

struct VisualiserConfig {
	std::string name = "";
	std::string fullPath = "";
};

class ConfigManager {
public:
	static void configToText(VisualiserConfig& in, std::vector<std::string>& out) {
		out.push_back(in.name);
		out.push_back(in.fullPath);
	}
	static void textToConfig(std::vector<std::string>& in, VisualiserConfig& out) {
		Vengine::warning("No error checking on text to config yet");
		out.name = in[0];
		out.fullPath = in[1];
	}
};