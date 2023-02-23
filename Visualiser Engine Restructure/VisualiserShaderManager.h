#pragma once
#include "VisualiserShader.h"
#include <unordered_map>
#include <set>
#include <vector>
#include <GL/glew.h>
#include <functional>
#include <string>

struct SSBOinfo {
	GLuint id;
	float* data;
	int dataLength;
};


class VisualiserShaderManager
{
public:

	static void setCurrentVisualiser(std::string currentVisualiserPath) { _currentVisualiserPath = currentVisualiserPath; }
	static void add(std::string shaderPath, int& id);
	static void updateDynamicSSBOs();
	static void updateAugmentedShaders();

	//ssbo managing--
	static bool initStaticSSBO(int bindingId, float* staticData, int dataLength);
	static bool initDynamicSSBO(int bindingId, std::function<float*()> updaterFunction, int dataLength);

	static void eraseSSBO(int bindingId);
	static void changeUpdaterFunctionForDynamicSSBO(int bindingId, std::function<float* ()> newUpdater);
	//--

	static bool SSBOexists(int bindingId);
private:

	static std::string _currentVisualiserPath;
	static std::unordered_map<int, VisualiserShader> _shaderCache;

	//binding is the external id for accessing ssbo
	static std::unordered_map<int, SSBOinfo> _SSBOinfoMap;
	static std::unordered_map<int, std::function<float*()>> _updaterFunctionMap;

	static std::set<int> _currentBindings;

	static bool SSBOinitPossible(); //error checker
	static int getNextAvailiableBinding(); //lowest unused ssbo binding

};

