#pragma once
#include "GLtexture.h"
#include <string>
#include <vector>

class IOManager//static class
{
public:
	static GLtexture loadPNG(const std::string& filepath);
	static bool loadWAV(const std::string& filepath, std::vector<float>& buffer, int& outSampleRate);
private:
	static bool readFileToBuffer(const std::string& filepath, std::vector<unsigned char>& buffer);

	static int decodeWAV(std::vector<unsigned char>& in, std::vector<float>& out, int& outSampleRate);
	static bool getPartOfFile(std::vector<unsigned char>& buffer, std::vector<unsigned char>& out, int start, int length, bool littleEndian = false);
	static void flipEndian(std::vector<unsigned char>& in);
	static std::string byteVecToString(std::vector<unsigned char>& vec, int start = 0, int length = 0);
	static int byteVecToInt(std::vector<unsigned char>& vec, int start = 0, int length = 0);
};

