#pragma once
#include <glm/glm.hpp>

#include <Vengine/Vengine.h>

#include <imgui.h>

class MyFuncs
{
public:
	static void setUniformsForShader(Vengine::GLSLProgram* shaderProgram);

	static void updateRMS(float rms);
private:
	static glm::vec2 _screenWH;
	static float _rms;
};

