#pragma once
#include <unordered_map>

#include "Visualiser.h"


class VisualiserManager
{
public:
	static bool createNewVisualiser(std::string name);
	static bool loadVisualiser(std::string path);

	static bool save();
	static bool saveAsNew(std::string name);
private:
	static Visualiser _current;
};

