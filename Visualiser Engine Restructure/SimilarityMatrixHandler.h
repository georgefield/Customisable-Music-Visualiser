#pragma once
#include "SelfSimilarityMatrix.h"
#include "FFTWapi.h"

class SimilarityMatrixHandler
{
public:
	SimilarityMatrixHandler(int size);
	~SimilarityMatrixHandler();

	void init(Master* master, int matrixSize);
	void reInit(int matrixSize);

	void calculateNext();

	void linkToMFCCs(int coeffLow, int coeffHigh);
	void linkToMelBandEnergies();
	void linkToMelSpectrogram();
	void linkToFourierTransform(float cutoffLow = -1.0f, float cutoffHigh = -1.0f, float smoothFactor = 0.0f);

	SelfSimilarityMatrix matrix;

	//getters
	bool isRealTime() { return _samplesAheadForFutureMatrix == 0; }
	LinkedTo isLinkedTo() { return _linkedTo; }
	bool isLinkInfoTheSame(int coeffLow, int coeffHigh) { return _linkedTo == MFCC && coeffLow == _coeffLow && coeffHigh == _coeffHigh; }	
	bool isLinkInfoTheSame(float cutoffLow, float cutoffHigh, float cutoffSmoothFactor) { return _linkedTo == FT && cutoffLow == _cutoffLow && cutoffHigh == _cutoffHigh && _cutoffSmoothFactor == cutoffSmoothFactor; }

private:

	Master* _master;
	Master _futureMaster;
	MFCCs _futureMFCCs;

	LinkedTo _linkedTo;
	FourierTransform* _fourierTransform;

	int _samplesAheadForFutureMatrix;
	int _matrixSize;

	//link info--
	int _coeffLow;
	int _coeffHigh;

	float _cutoffLow;
	float _cutoffHigh;
	float _cutoffSmoothFactor;
	//--

	void initSetters();
	void deleteSetters();
};

