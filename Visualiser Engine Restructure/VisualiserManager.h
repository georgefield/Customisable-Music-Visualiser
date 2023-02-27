#pragma once
#include <unordered_map>
#include <functional>
#include <vector>

#include "Visualiser.h"

class VisualiserManager
{
public:
	static void init();

	static bool createNewVisualiser(std::string name);
	static bool loadVisualiser(std::string path);

	static bool save();
	static bool saveAsNew(std::string name);

	static std::string externalToInternalTexture(std::string texturePath);

	static bool isVisualiserLoaded() { return _current.isInitialised(); }
	static std::string path() { return _current.getPath(); }
	static std::string shadersFolder() { return _current.getPath() + "/shaders"; }
	static std::string texturesFolder() { return _current.getPath() + "/textures"; }


private:
	static Visualiser _current;
};

