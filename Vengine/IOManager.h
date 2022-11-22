#pragma once
#include "GLtexture.h"
#include <string>
#include <vector>
#include <SDL/SDL.h>

namespace Vengine {

	class IOManager//static class
	{
	public:
		static GLtexture loadPNG(const std::string& filepath);
		static bool readFileToBuffer(const std::string& filepath, std::vector<unsigned char>& buffer);
		static void getFilesInDir(const std::string& dirPath, std::vector<std::string>& files);
	};

}