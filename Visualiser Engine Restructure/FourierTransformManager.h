#pragma once
#include "FourierTransform.h"
#include <unordered_map>
#include <GL/glew.h>
#include <string>

class FourierTransformManager
{
public:
	//setup manager
	static void setMaster(Master* master);

	//managing
	static void createFourierTransform(int& id, int historySize, float cutOffLow = 0.0f, float cutOffHigh = 22050.0f, float cutoffSmoothFrac = 0.0f);

	static bool fourierTransformExists(int id);

	static void eraseFourierTransform(int id);

	static FourierTransform* getFourierTransform(int id);

	static int numTransforms() { return _fourierTransforms.size(); }
	static std::vector<int> idArr();

private:
	//called on create
	static void addUniformSetterFunctionOptionsToList(int id);
	static void addSSBOsetterFunctionOptionsToList(int id);
	//called on erase
	static void deleteUniformSetterFunctionOptionsFromList(int id);
	static void deleteSSBOsetterFunctionOptionsFromList(int id);

	static std::unordered_map<int, FourierTransform*> _fourierTransforms;
	static Master* _master;
};