#include "IOManager.h"
#include "picoPNG.h"
#include "MyErrors.h"

#include <fstream>
#include <filesystem>
#include <direct.h>
#include <iostream>


using namespace Vengine;


GLtexture IOManager::loadPNG(const std::string& filepath) {
	GLtexture texture = {}; //init all to 0

	std::vector<unsigned char> in;
	std::vector<unsigned char> out;
	unsigned long width, height;

	if (readFileToBuffer(filepath, in) == false) {
		fatalError("failed to load PNG file " + filepath + " to buffer");
	}

	int errorCode = decodePNG(out, width, height, &(in[0]), in.size()); //picoPNG decoder

	if (errorCode != 0) {
		fatalError("decodePNG failed with error: " + std::to_string(errorCode));
	}

	///-- generate id, bind id to texture_2D, upload image to vram, set parameters, unbind texture_2D
	glGenTextures(1, &(texture.id)); //generate id for texture
	glBindTexture(GL_TEXTURE_2D, texture.id); //bind texture id to GL_TEXTURE_2D
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(out[0])); //upload to GPU (stored in VRAM)

	//settings for displaying texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //linear interpolation if resized
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //^^^
	glGenerateMipmap(GL_TEXTURE_2D); //mipmap generates smaller images for display texture if its scaled down

	glBindTexture(GL_TEXTURE_2D, 0); //unbind texture id from GL_TEXTURE_2D
	///--

	//set texture values (found by decodePNG)
	texture.width = width;
	texture.height = height;
	return texture; //return copy of texture
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

bool IOManager::readTextFileToBuffer(const std::string& filepath, std::vector<std::string>& buffer) {

	std::ifstream file(filepath);
	if (file.fail()) {
		perror(filepath.c_str());
		return false;
	}

	if (!file.is_open()) {
		Vengine::warning("Failed to open file " + filepath);
		return false;
	}

	//line by line in vector
	buffer.clear();
	std::string tmp = "";
	while (std::getline(file, tmp)) {
		buffer.push_back(tmp);
	}

	return true;
} //for ascii mainly

bool Vengine::IOManager::outputTextFile(const std::string& filepath, std::vector<std::string>& fileContents){

		std::ofstream file(filepath); // create file object
		if (file.is_open()) { // check if file was successfully opened
			for (const auto& content : fileContents) {
				file << content << "\n"; // write each string in vector to file with newline character
			}
			file.close(); // close file
			return true;
		}

		Vengine::warning("Was not able to create/open then output to file " + filepath);
		return false;
}

bool Vengine::IOManager::clearTextFile(const std::string& filepath)
{
	if (!std::filesystem::exists(filepath)) {
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
	for (const auto& entry : std::filesystem::directory_iterator(dirPath)){
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


void IOManager::copyDirectory(const std::string& source, const std::string& destination) {
	const std::filesystem::path sourcePath(source);
	const std::filesystem::path destinationPath(destination);

	if (!std::filesystem::exists(destinationPath)) {
		std::filesystem::create_directory(destinationPath);
	}

	for (const auto& entry : std::filesystem::directory_iterator(sourcePath)) {
		const auto& path = entry.path();
		const auto& newPath = destinationPath / path.filename();
		if (std::filesystem::is_directory(path)) {
			copyDirectory(path.string(), newPath.string());
		}
		else {
			std::filesystem::copy_file(path, newPath, std::filesystem::copy_options::overwrite_existing);
		}
	}
}

std::string Vengine::IOManager::getProjectDirectory()
{
	return std::filesystem::current_path().generic_string();
}

