#pragma once
#include "GLSLProgram.h"
#include <iostream>
#include <string>

namespace Vengine {

	struct ShaderInfo {
		ShaderInfo(std::string VertPath, std::string FragPath) {
			vertPath = VertPath;
			fragPath = FragPath;
		}

		~ShaderInfo() {
			if (program != nullptr) {
				delete program;
			}
		}

		void createProgramObject() {
			program = new GLSLProgram();

			program->compileShaders(vertPath, fragPath);
			//hard coded attribs
			program->addAttrib("vertexPosition");
			program->addAttrib("vertexColour");
			program->addAttrib("vertexUV");

			program->linkShaders();

			program->updateShaderUniformInfo();
		}

		std::string vertPath;
		std::string fragPath;

		GLSLProgram* program = nullptr;
	};

	class ShaderCache
	{
	public:
		ShaderCache();
		~ShaderCache();

		GLSLProgram* getProgram(std::string vertPath, std::string fragPath);
		void unloadAll();

	private:
		std::vector<ShaderInfo*> _loadedShaders;
		std::vector<ShaderInfo*>::iterator findShader(std::string vertPath, std::string fragPath);
	};

}
