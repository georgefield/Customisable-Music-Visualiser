#pragma once
#include "GLSLProgram.h"

#include <string>
#include <map>

namespace Vengine {

	class ShaderCache
	{
	public:
		ShaderCache();
		~ShaderCache();

		GLSLProgram* getProgram(std::string shaderFilepath);

	private:
		std::map<std::string, GLSLProgram*> _shaderMap;
	};

}
