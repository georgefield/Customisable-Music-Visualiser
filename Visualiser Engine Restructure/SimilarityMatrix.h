#pragma once
#include "VectorHistory.h"
#include <Vengine/MyErrors.h>
#include "MyMaths.h"
#include "DataTextureCreator.h"
#include "VisVars.h"

class SimilarityMatrix {
public:
	SimilarityMatrix(int matWidth, int vectorDim);
	~SimilarityMatrix();

	void add(float* v, float contrastFactor);

	float getSimilarityMeasureThreaded();
	float getSimilarityMeasure();
	History<float>* getSimilarityMeasureHistory() { return &_similarityMeasureHistory; }

	//texture manip
	int textureStartIndex() { return _start; }
	bool isTextureCreated() { return _textureCreator.isCreated(); }
	void createTexture() { _textureCreator.createTexture(_matrixWidth, _matrixWidth, _data); }
	void deleteTexture() { _textureCreator.deleteTexture(); }
	Vengine::GLtexture getMatrixTexture() { 
		if (isTextureCreated()) return _textureCreator.getTexture();
		createTexture(); return _textureCreator.getTexture();
	} //creates a texture if one doesnt exist

	//matrix info getters
	float get(int i, int j) { return _data[toDataIndex(i,j)]; }
	float* getData() { return _data; }
	int getDataStart() { return _start; }
	int matrixSize() const { return _matrixWidth; }
	bool full() { return (_vectorHistory.entries() == _vectorHistory.totalSize()); }
	int entries() { return _vectorHistory.entries(); }

private:
	int _matrixWidth;
	int _dataLength;
	int _start;

	float* _data; //access via coordinates using the 'toDataIndex' function
	int _vectorDim;
	VectorHistory<float> _vectorHistory;
	History<float> _vectorMagnitudeHistory;

	History<float> _similarityMeasureHistory;

	DataTextureCreator _textureCreator;

	//get data index from coordinates
	inline int toDataIndex(int i, int j) {
		return ((_start + i) % _matrixWidth) * _matrixWidth + ((_start + j) % _matrixWidth);
	}

	//similarity measure calculation functions--
	inline float checkerboardKernel(int i, int j);
	//use struct to make error detection easier
	struct simMeasurePartInfo {
		simMeasurePartInfo(int _jStart, int _jEnd, float& _out) :
			jStart(_jStart),
			jEnd(_jEnd),
			out(_out)
		{}
		int jStart;
		int jEnd;
		float& out;
	};
	void getSimMeasurePart(const simMeasurePartInfo& info);

	//--
};
