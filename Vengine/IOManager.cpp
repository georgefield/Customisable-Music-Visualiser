#include "IOManager.h"
#include "picoPNG.h"
#include "Errors.h"

#include <fstream>
#include <filesystem>


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
}

void IOManager::getFilesInDir(const std::string& dirPath, std::vector<std::string>& files)
{
	files.clear();
	for (const auto& entry : std::filesystem::directory_iterator(dirPath)){
		files.push_back(entry.path().stem().string());
	}
}
