#pragma once
#include <string>
struct VisPaths {
	//resource paths--

	static const std::string _defaultFragShaderPath;
	static const std::string _commonVertShaderPath;
	static const std::string _shaderPrefixPath;
	static const std::string _uniformNameScraperShaderPath;
	static const std::string _1x1WhiteTexturePath;

	//visualiser folder paths--

	static const std::string _userCreatedVisualiserPath;
	static const std::string _presetVisualiserPath;
	static const std::string _startupVisualiserPath;

	//other--

	static const std::string _visShaderExtension;
};