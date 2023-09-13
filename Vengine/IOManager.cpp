#include "IOManager.h"
#include "MyErrors.h"

#include <fstream>
#include <filesystem>
#include <direct.h>
#include <iostream>


using namespace Vengine;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


GLtexture Vengine::IOManager::loadImage(const std::string& filepath)
{
	GLtexture texture = {}; //init all to 0
	int width = -1, height = -1;

	///stb image.h implementation--
	int composition; //how many byte make up a pixel
	const int forceComposition = 4; //force 4 byte pixels (0 if dont force)

	stbi_set_flip_vertically_on_load(true); //flip image y to work better with openGL where (0,0) is BL instead of TL
	unsigned char* imageDataPtr = stbi_load(filepath.c_str(), &width, &height, &composition, forceComposition);

	if (imageDataPtr == NULL) { //failed image loading
		warning(stbi_failure_reason());
		return {};
	}

	if (composition != forceComposition && forceComposition != 0) { //changed pixel byte count (show warning)
		warning("Composition of image " + filepath + " forcibly changed from " + std::to_string(composition) + " bytes per pixel to " + std::to_string(forceComposition));
	}
	///--

	///-- generate id, bind id to texture_2D, upload image to vram, set parameters, unbind texture_2D
	glGenTextures(1, &(texture.id)); //generate id for texture
	glBindTexture(GL_TEXTURE_2D, texture.id); //bind texture id to GL_TEXTURE_2D
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageDataPtr); //upload to GPU (stored in VRAM)

	//settings for displaying texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //linear interpolation if resized
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //^^^
	glGenerateMipmap(GL_TEXTURE_2D); //mipmap generates smaller images for display texture if its scaled down

	glBindTexture(GL_TEXTURE_2D, 0); //unbind texture id from GL_TEXTURE_2D
	///--

	stbi_image_free(imageDataPtr); //free the image data from heap after uploading to VRAM

	//set texture values (found by decodePNG)
	texture.width = width;
	texture.height = height;
	return texture; //return copy of texture



	return GLtexture();
}


bool IOManager::readFileToBuffer(const std::string& filepath, std::vector<unsigned char>& buffer) {

	std::ifstream file(filepath, std::ios::binary); //binary flag
	if (file.fail()) {
		perror(filepath.c_str());
		return false;
	}

	//seek to end
	file.seekg(0, std::ios::end);

	//get file size
	int fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	fileSize -= file.tellg();

	buffer.resize(fileSize);
	file.read((char*)&(buffer[0]), fileSize); //pretends were reading to char array, doesnt matter as reading binary
	return true;
} //for binary mainly


bool IOManager::copyFile(const std::string& sourceFile, const std::string& destinationFile)
{
	if (fileExists(destinationFile)) {
		Vengine::warning("Cannot copy file '" + sourceFile + "' as destination file already exists");
		return false;
	}
	try {
		std::filesystem::copy_file(sourceFile, destinationFile);
		return true;
	}
	catch (const std::exception& e) {
		Vengine::warning(e.what());
		Vengine::warning("Failed to copy file");
	}
	return false;
}

bool Vengine::IOManager::fileExists(const std::string& filename)
{
	return std::filesystem::exists(filename);
}


bool IOManager::readTextFileToString(const std::string& filepath, std::string& buffer) {

	std::ifstream file(filepath);
	if (file.fail()) {
		perror(filepath.c_str());
		return false;
	}

	if (!file.is_open()) {
		Vengine::warning("Failed to open file " + filepath);
		return false;
	}

	std::string tmp;
	while (std::getline(file, tmp)) {
		buffer += tmp + "\n";
	}

	return true;
}


bool Vengine::IOManager::readTextFileToVector(const std::string& filepath, std::vector<std::string>& buffer) {

	std::ifstream file(filepath);
	if (file.fail()) {
		perror(filepath.c_str());
		return false;
	}

	if (!file.is_open()) {
		Vengine::warning("Failed to open file " + filepath);
		return false;
	}

	std::string tmp;
	while (std::getline(file, tmp)) {
		buffer.push_back(tmp);
	}

	return true;
}


bool Vengine::IOManager::outputToTextFile(const std::string& filepath, const std::string& fileContents, bool wipeExisting) {
	
	std::ofstream file;
	if (wipeExisting) //if wipe existing add truncate flag
		file.open(filepath, std::ios::out | std::ios::trunc);
	else
		file.open(filepath, std::ios::out);

	if (file.is_open()) { // check if file was successfully opened
		file << fileContents; // write to file
		file.close(); // close file
		return true;
	}

	Vengine::warning("Was not able to create/open then output to file " + filepath);
	return false;
}

bool Vengine::IOManager::clearFile(const std::string& filepath)
{
	if (!fileExists(filepath)) {
		Vengine::warning("Tried to clear text file that does not exist");
		return false;
	}
	std::ofstream file(filepath, std::ios::trunc); //trunc flag removes all preexisting data
	file.close();
	return true;
}

void IOManager::getFilesInDir(const std::string& dirPath, std::vector<std::string>& files, bool showExtension, std::string extension)
{
	files.clear();
	for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
		if (extension == "" || entry.path().extension().string() == extension) {
			files.push_back(entry.path().stem().string() + (showExtension ? entry.path().extension().string() : ""));
		}
	}
}

bool IOManager::createFolder(const std::string& folderPath, bool isHidden = false) {
	int status;
	if (isHidden) {
		status = _mkdir((folderPath + ".").c_str());
	}
	else {
		status = _mkdir(folderPath.c_str());
	}

	if (status == 0) {
		return true;
	}
	else {
		Vengine::warning("Was not able to create folder " + folderPath);
		return false;
	}
}

bool IOManager::directoryExists(std::string path) {
	std::filesystem::path directoryPath = path;

	return std::filesystem::exists(directoryPath) && std::filesystem::is_directory(directoryPath);
}


bool IOManager::copyDirectory(const std::string& source, const std::string& destination) {
	const std::filesystem::path sourcePath(source);
	const std::filesystem::path destinationPath(destination);

	if (!std::filesystem::exists(destinationPath)) {
		std::filesystem::create_directory(destinationPath);
	}

	for (const auto& entry : std::filesystem::directory_iterator(sourcePath)) {
		const auto& path = entry.path();
		const auto& newPath = destinationPath / path.filename();
		if (std::filesystem::is_directory(path)) {
			if (!copyDirectory(path.string(), newPath.string())) {
				return false;
			}
		}
		else {
			try {
				std::filesystem::copy_file(path, newPath, std::filesystem::copy_options::overwrite_existing);
			}
			catch (const std::exception& e) {
				Vengine::warning(e.what());
				Vengine::warning("Failed to copy file " + path.string() + " when copying directory");
				return false;
			}
		}
	}
	return true;
}

bool IOManager::isInParentDirectory(const std::string& parentDirectory, const std::string& path)
{
	std::filesystem::path parentPath(parentDirectory);
	std::filesystem::path filePath(path);

	// check if filePath is an absolute path
	if (!filePath.is_absolute()) {
		filePath = std::filesystem::absolute(filePath);
	}

	// get the canonical paths of both parent and filePath
	parentPath = std::filesystem::canonical(parentPath);
	filePath = std::filesystem::canonical(filePath);

	// check if filePath is in parent directory
	return filePath.string().find(parentPath.string()) == 0;
}

std::string Vengine::IOManager::getProjectDirectory()
{
	return std::filesystem::current_path().generic_string();
}

