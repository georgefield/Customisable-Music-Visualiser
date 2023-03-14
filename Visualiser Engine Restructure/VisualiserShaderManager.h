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
	static void init();

	static void setCurrentVisualiser(std::string currentVisualiserPath) { _currentVisualiserPath = currentVisualiserPath; }

	static VisualiserShader* getShader(std::string fragPath);

	static std::string getCommonShaderPrefix();

	static bool createShader(std::string name);
	//--

	//ssbo managing--
	struct SSBOs {
		static void updateDynamicSSBOs();

		static void setSSBOupdater(std::string SSBOname, std::function<float* ()> function, int dataLength);
		static void setSSBOupdater(std::string SSBOname, float* staticDataPtr, int dataLength);
		static void initialiseSSBO(int binding);

		static void removeSSBOupdater(std::string SSBOname);

		static void getAllSSBOnames(std::vector<std::string>& names);
		static std::string getSSBOstatus(std::string name, float* dataOut = nullptr, int* dataLength = nullptr);
	};
	//--

	//uniform setter function managing--
	struct Uniforms {
		//can only set with function now to allow for variable changes
		static void setUniformUpdater(std::string uniformName, std::function<float()> function);
		static void setUniformUpdater(std::string uniformName, std::function<int()> function); //overload for int dynamic functions

		static void removeUniformUpdater(std::string uniformName);

		static void getAllUniformNames(std::vector<std::string>& names, GLenum type = NULL);
		static float getUniformValue(std::string uniformName, GLenum type = NULL);
		static float getUniformValue(std::string uniformName, GLenum* type = NULL);
	};
	//--

	static void setShaderUniforms(VisualiserShader* shader);
	static void updateUniformValuesToOutput();

private:

	static std::string _currentVisualiserPath;

	static GLuint _userVarsSSBOid;
	
	//contains all loaded visualiser shaders
	//key accessing shader is the fragPath
	static std::unordered_map<std::string, VisualiserShader> _shaderCache;

	//key for accessing default ssbos is ssbo binding
	static std::unordered_map<int, SSBOsetter> _SSBOupdaterMap;
	static std::unordered_map<std::string, int> _defaultSSBOnamesAndBindings;

	//ket for accessing default uniform setters is uniform name
	static std::unordered_map<std::string, UniformSetter<float>> _floatUniformUpdaterMap;
	static std::unordered_map<std::string, UniformSetter<int>> _intUniformUpdaterMap;
	static std::unordered_map<std::string, GLenum> _defaultUniformNames;



	static bool inSSBOmap(int binding);
	static bool isVisSSBO(std::string SSBOname);

	static bool inUniformMap(std::string uniformName, GLenum* type);
	static bool inUniformMap(std::string uniformName, GLenum type = NULL);
	static bool isVisUniform(std::string uniformName, GLenum* type);
	static bool isVisUniform(std::string uniformName, GLenum type = NULL);
};

