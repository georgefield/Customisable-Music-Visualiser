#pragma once
#include <unordered_map>
#include <set>
#include <vector>
#include <GL/glew.h>
#include <functional>
#include <string>


#include "VisualiserShader.h"
#include "History.h"
#include "SSBOsetter.h"



class VisualiserShaderManager
{
public:

	//visualiser shader managing--
	static void setCurrentVisualiser(std::string currentVisualiserPath) { _currentVisualiserPath = currentVisualiserPath; }

	static VisualiserShader* getShader(std::string fragPath);
	static std::string getDefaultFragmentShaderPath();

	static void getUsableShaderFragPaths(std::vector<std::string>& shaderPaths);
	//--

	//ssbo managing--
	struct SSBOs {
		static void updateDynamicSSBOs();

		static void addPossibleSSBOSetter(std::string functionName, std::function<float*()> function, int dataLength);
		static void addPossibleSSBOSetter(std::string functionName, float* staticData, int dataLength);
		static void deleteSSBOsetter(std::string functionName);

		static bool setSSBO(int bindingId, std::string functionName);
		static void unsetSSBO(int bindingId);
		static bool SSBOisSet(int bindingId);

		static void getAvailiableBindings(std::vector<int>& availiableBindings);
		static void getSetBindings(std::vector<int>& setBindings);

		static void getSSBOsetterNames(std::vector < std::string>& names);
		static SSBOsetter getSSBOsetter(std::string name);
		static std::string getSSBOsetterName(int bindingID);
	};
	//--

	//uniform setter function managing--
	struct Uniforms {
		//can only set with function now to allow for variable changes
		static void addPossibleUniformSetter(std::string functionName, std::function<float()> function);
		static void addPossibleUniformSetter(std::string functionName, std::function<int()> function); //overload for int dynamic functions

		static void deletePossibleUniformSetter(std::string functionName);
		static void getUniformSetterNames(std::vector<std::string>& names);
		static void getFloatUniformSetterNames(std::vector<std::string>& names);
		static void getIntUniformSetterNames(std::vector<std::string>& names);

		static UniformSetter<float> getFloatUniformSetter(std::string functionName);
		static UniformSetter<int> getIntUniformSetter(std::string functionName);
	};
	//--

	static void addHistoryAsPossibleSSBOsetter(std::string historyName, History<float>* history);
	static void deleteHistoryAsPossibleSSBOsetter(std::string historyName);
private:

	static std::string _currentVisualiserPath;
	
	//contains all loaded visualiser shaders
	//key accessing shader is the fragPath
	static std::unordered_map<std::string, VisualiserShader> _shaderCache;

	//ssbo managing--
	//key for accessing ssbo is ssbo binding
	static std::unordered_map<int, SSBOsetter> _setSSBOinfoMap;
	static std::unordered_map<std::string, SSBOsetter> _SSBOsetterMap;

	static bool SSBOinitPossible(); //error checker
	//--

	//uniform setter function managing--
	//possible functions to be linked to uniforms
	//key is function name
	static std::unordered_map<std::string, UniformSetter<float>> _floatFunctions;
	static std::unordered_map<std::string, UniformSetter<int>> _intFunctions;
	//--
};

