#pragma once
#include "GLSLProgram.h"
#include "TextureCache.h"
#include "ShaderCache.h"

#include <map>
#include <vector>
#include <GL/glew.h>

namespace Vengine {

	struct myComplex {
		float real = 0;
		float imag = 0;

		bool equals(myComplex test) {
			if (real == test.real && imag == test.imag) {
				return true;
			}
			return false;
		}
	};

	class ResourceManager
	{
	public:
		static GLtexture getTexture(std::string textureFilepath); //loads texture in aswell
		static GLSLProgram* getShaderProgram(std::string shaderName); //loads shader in aswell

		static void getFFT(std::vector<float>& samples, int currentSample, std::vector<float>& harmonicValues);
	private:
		static TextureCache _textureCache;
		static ShaderCache _shaderCache;

		//static std::map<GLuint, GLSLProgram> _programCache;
	};

}