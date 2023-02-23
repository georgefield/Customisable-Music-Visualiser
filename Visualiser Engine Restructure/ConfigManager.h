#pragma once

struct VisualiserConfig {
	std::string var1;
	std::string var2;
};

class ConfigManager {
public:
	static void configToText(VisualiserConfig& in, std::vector<std::string>& out) {
		out.push_back(in.var1);
		out.push_back(in.var2);
	}
	static void textToConfig(std::vector<std::string>& in, VisualiserConfig& out) {
		Vengine::warning("No error checking on text to config yet");
		out.var1 = in[0];
		out.var2 = in[1];
	}
};