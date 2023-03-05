#pragma once
#include "SelfSimilarityMatrix.h"
#include "FFTWapi.h"

class FutureSimilarityMatrix
{
public:
	FutureSimilarityMatrix(int size);
	~FutureSimilarityMatrix();

	void init(int matrixSize);
	void reInit(int matrixSize);

	void calculateNext();

	void linkToMFCCs(int coeffLow, int coeffHigh);
	void linkToFourierTransform(float cutoffLow = -1.0f, float cutoffHigh = -1.0f, float smoothFrac = 0.0f);

	SelfSimilarityMatrix matrix;

private:
	static enum LinkedTo {
		NONE,
		MFCC,
		FT
	};

	Master _futureMaster;
	MFCCs _futureMFCCs;

	LinkedTo _linkedTo;
	FourierTransform* _fourierTransform;

	int _samplesAhead;
	int _matrixSize;
};

