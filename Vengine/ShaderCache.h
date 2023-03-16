#pragma once
#include "GLSLProgram.h"
#include <iostream>
#include <string>

namespace Vengine {

	struct ShaderInfo {
		ShaderInfo(std::string VertPath, std::string FragPath) {
			vertPath = VertPath;
			fragPath = FragPath;

			isSyntaxError = false;
			error = "";
		}

		~ShaderInfo() {
			if (program != nullptr) {
				delete program;
			}
		}

		void createProgramObject() {
			program = new GLSLProgram();

			if (!program->compileShaders(vertPath, fragPath)) {
				isSyntaxError = true;
				error = program->getSyntaxError();
				return;
			}
			//hard coded attribs
			program->addAttrib("vertexPosition");
			program->addAttrib("vertexColour");
			program->addAttrib("vertexUV");

			if (!program->linkShaders()) {
				isSyntaxError = true;
				error = program->getSyntaxError();
				return;
			}

			program->updateShaderUniformInfo();
		}

		std::string vertPath;
		std::string fragPath;

		bool isSyntaxError;
		std::string error;

		GLSLProgram* program = nullptr;
	};

	class ShaderCache
	{
	public:
		ShaderCache();
		~ShaderCache();

		GLSLProgram* getProgram(std::string vertPath, std::string fragPath, std::string& errorOut);
		void deleteProgram(std::string vertPath, std::string fragPath);
		void unloadAll();

	private:
		std::vector<ShaderInfo*> _loadedShaders;
		std::vector<ShaderInfo*>::iterator findShader(std::string vertPath, std::string fragPath);
	};

}
