#pragma once
#include "GLtexture.h"
#include <string>
#include <vector>
#include <SDL/SDL.h>


namespace Vengine {

	class IOManager//static class
	{
	public:
		static GLtexture loadImage(const std::string& filepath);

		static bool readFileToBuffer(const std::string& filepath, std::vector<unsigned char>& buffer);
		static bool copyFile(const std::string& sourceFile, const std::string& destinationFile);
		static bool fileExists(const std::string& filename);

		static bool readTextFileToString(const std::string& filepath, std::string& buffer);
		static bool readTextFileToVector(const std::string& filepath, std::vector<std::string>& buffer);
		static bool outputToTextFile(const std::string& filepath, const std::string& fileContents, bool wipeExisting);
		static bool clearFile(const std::string& filepath);

		static void getFilesInDir(const std::string& dirPath, std::vector<std::string>& files, bool showExtension = true, std::string extension = "");

		static bool createFolder(const std::string& folderPath, bool isHidden);
		static bool directoryExists(std::string path);
		static bool copyDirectory(const std::string& source, const std::string& destination);
		static bool isInParentDirectory(const std::string& parentDirectory, const std::string& path);

		static std::string getProjectDirectory();

	};

}