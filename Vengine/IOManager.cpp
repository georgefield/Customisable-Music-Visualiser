#include "IOManager.h"
#include "picoPNG.h"
#include "Errors.h"

#include <fstream>

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

bool IOManager::loadWAV(const std::string& filepath, std::vector<float>& buffer, int& outSampleRate) {

	std::vector<unsigned char> in;
	if (readFileToBuffer(filepath, in) == false) {
		fatalError("failed to load WAV file " + filepath + " to buffer");
	}

	int errorCode = decodeWAV(in, buffer, outSampleRate);

	if (errorCode != 0) {
		fatalError("decodeWAV failed with error: " + std::to_string(errorCode));
	}
	return true;
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



///---OBSELETE CODE - USING SDL INCLUDED WAV DECODER (IN AUDIO.cpp)

//error codes
const int NOERROR = 0;
const int INCORRECTHEADER = 1;
const int INCORRECTCHUNKHEADER1 = 2;
const int INCORRECTCHUNKHEADER2 = 3;
const int NODATA = 4;
const int BADDATA = 5;

int IOManager::decodeWAV(std::vector<unsigned char>& in, std::vector<float>& out, int& outSampleRate) {

	if (byteVecToString(in, 0, 4) != "RIFF") { return INCORRECTHEADER; }
	if (byteVecToString(in, 8, 4) != "WAVE") { return INCORRECTHEADER; }

	if (byteVecToString(in, 12, 3) != "fmt") { return INCORRECTCHUNKHEADER1; }

	std::vector<unsigned char> workingVec;
	int numChannels; //1 for mono, 2 for stereo
	int sampleRate; //samples of audio per second
	int byteRate; //bytes per second
	int blockAlign; //num bytes for one sample including all channels
	int bitsPerSample; //how many bits for each sample e.g. 8 bit 16 bit

	if (!getPartOfFile(in, workingVec, 22, 2, true)) { return NODATA; }
	numChannels = byteVecToInt(workingVec);

	if (!getPartOfFile(in, workingVec, 24, 4, true)) { return NODATA; }
	sampleRate = byteVecToInt(workingVec);

	if (!getPartOfFile(in, workingVec, 28, 4, true)) { return NODATA; }
	byteRate = byteVecToInt(workingVec);

	if (!getPartOfFile(in, workingVec, 32, 2, true)) { return NODATA; }
	blockAlign = byteVecToInt(workingVec);
	printf("%i, block align", blockAlign);
	system("PAUSE");

	if (!getPartOfFile(in, workingVec, 34, 2, true)) { return NODATA; }
	bitsPerSample = byteVecToInt(workingVec);

	//sanity check
	if (byteRate != sampleRate * numChannels * bitsPerSample / 8) {
		return BADDATA;
	}
	if (blockAlign != numChannels * bitsPerSample / 8) {
		return BADDATA;
	}

	if (byteVecToString(in, 36, 4) != "data") { return INCORRECTCHUNKHEADER2; }

	int dataSize; //from 44 offset, in bytes, = num samples * num channels * bits per sample / 8
	getPartOfFile(in, workingVec, 40, 4, true);
	dataSize = byteVecToInt(workingVec);

	outSampleRate = sampleRate;
	printf("sample rate: %i\n", sampleRate);

	//add more parameters to function---------
	//temporary grab of mono left audio
	out.resize(dataSize);

	int it = 0;
	while (it < dataSize) {
		getPartOfFile(in, workingVec, it + 44, bitsPerSample / 8, true);
		int toAdd = byteVecToInt(workingVec);
		if ((toAdd & (1 << (bitsPerSample - 1))) >> (bitsPerSample - 1)) { //fix twos complement (hacky)
			toAdd -= pow(2, bitsPerSample);
		}
		out.push_back(((float)toAdd) / (pow(2, bitsPerSample - 1))); //normalises to between -1 & 1 before returning
		it += blockAlign;
	}
	//---------- make above code include stereo options & such
	return 0;
}

///---



bool IOManager::getPartOfFile(std::vector<unsigned char>& buffer, std::vector<unsigned char>& out, int start, int length, bool littleEndian) {

	if (start + length > buffer.size()) {
		printf("attempting to read memory past file buffer length\n");
		out.resize(0);
		return false;
	}

	out.resize(length); //resize, all values will be overwritten so dont need to clear

	for (int i = 0; i < length; i++) {
		out[i] = buffer[start + i];
	}
	if (littleEndian) {
		flipEndian(out);
	}
	return true;
}

void IOManager::flipEndian(std::vector<unsigned char>& in) {
	if (in.size() > 4) {
		printf("WARNING: flipping endian of data size > 4 probably not correct");
	}
	unsigned char tmp;
	for (int i = 0; i < (in.size() / 2); i++) {
		tmp = in[i];
		in[i] = in[in.size() - i - 1];
		in[in.size() - i - 1] = tmp;
	}
}

std::string IOManager::byteVecToString(std::vector<unsigned char>& vec, int start, int length) {
	if (length == 0) {
		length = vec.size();
	}
	std::string ret = "";
	for (int i = start; i < start + length; i++) {
		ret += vec.at(i);
	}
	return ret;
}

int IOManager::byteVecToInt(std::vector<unsigned char>& vec, int start, int length) {
	if (length == 0) {
		length = vec.size();
	}
	if (length > 4 or length < 0) {
		fatalError("can only convert 4 bytes or less to integer");
	}
	int ret = 0;
	for (int i = start; i < start + length; i++) {
		ret |= vec.at(i) << 8 * ((start + length) - i - 1);
	}
	return ret;
}