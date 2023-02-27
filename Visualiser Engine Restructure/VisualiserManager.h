#pragma once
#include <unordered_map>
#include <functional>
#include <vector>

#include "Visualiser.h"
#include "UniformSetter.h"

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

	//functions to update uniforms with
	static void addPossibleUniformSetter(std::string name, std::function<float()> function);
	static void addPossibleUniformSetter(std::string name, std::function<int()> function); //overload for int dynamic functions
	static void addPossibleUniformSetter(std::string name, float constant); //<
	static void addPossibleUniformSetter(std::string name, int constant); //  < overloads for constant value initialisation 

	static void deletePossibleUniformSetter(std::string name);
	static void getFloatUpdaterFunctionNames(std::vector<std::string>& names);
	static void getIntUpdaterFunctionNames(std::vector<std::string>& names);

	static UniformSetter<float> getFloatSetterFunction(std::string name);
	static UniformSetter<int> getIntSetterFunction(std::string name);

private:
	static Visualiser _current;

	//possible functions to be linked to uniforms
	static std::unordered_map<std::string, UniformSetter<float>> _floatFunctions;
	static std::unordered_map<std::string, UniformSetter<int>> _intFunctions;
};

