#pragma once
#include <unordered_map>
#include <set>
#include <vector>
#include <GL/glew.h>
#include <functional>
#include <string>

#include "VisualiserShader.h"

struct SSBOinfo {
	GLuint id;
	float* data;
	int dataLength;
};


class VisualiserShaderManager
{
public:

	static void setCurrentVisualiser(std::string currentVisualiserPath) { _currentVisualiserPath = currentVisualiserPath; }
	static void updateDynamicSSBOs();
	static void updateShaderUniforms();

	//ssbo managing--
	static bool initStaticSSBO(int bindingId, float* staticData, int dataLength);
	static bool initDynamicSSBO(int bindingId, std::function<float*()> updaterFunction, int dataLength);

	static void eraseSSBO(int bindingId);
	static void changeUpdaterFunctionForDynamicSSBO(int bindingId, std::function<float* ()> newUpdater);

	static bool SSBOalreadyBound(int bindingId);
	//--

	static VisualiserShader* getShader(std::string fragPath);
	static std::string getDefaultFragmentShaderPath();
private:

	static std::string _currentVisualiserPath;
	
	//key accessing shader is the fragPath
	static std::unordered_map<std::string, VisualiserShader> _shaderCache;

	//ssbo managing--
	//key for accessing ssbo is ssbo binding
	static std::unordered_map<int, SSBOinfo> _SSBOinfoMap;
	static std::unordered_map<int, std::function<float*()>> _updaterFunctionMap;

	static std::set<int> _currentBindings;

	static bool SSBOinitPossible(); //error checker
	static int getNextAvailiableBinding(); //lowest unused ssbo binding
	//--


};

