#pragma once
#include "FourierTransform.h"
#include <unordered_map>
#include <GL/glew.h>
#include <string>

class FourierTransformManager
{
public:
	static void createFourierTransformFromStruct(FourierTransform::FTinfo info);

	static void createFourierTransform(int id, int historySize, float cutOffLow = 0.0f, float cutOffHigh = 22050.0f, float cutoffSmoothFrac = 0.0f);

	static bool fourierTransformExists(int id);

	static void eraseFourierTransform(int id);

	static void clearFourierTransforms();

	static void reInitAll();
	static void calculateFourierTransforms();

	static FourierTransform* getFourierTransform(int id);

	//info getters
	static int numTransforms() { return _fourierTransforms.size(); }
	static std::vector<int> idArr();
	static std::vector<int> availiableIdArr();

private:

	static std::unordered_map<int, FourierTransform*> _fourierTransforms;
};