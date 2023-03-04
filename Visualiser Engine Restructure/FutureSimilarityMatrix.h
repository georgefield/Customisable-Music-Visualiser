#pragma once
#include "SelfSimilarityMatrix.h"
#include "FFTWapi.h"

class FutureSimilarityMatrix
{
public:
	FutureSimilarityMatrix(int size);

	void init(int samplesAhead);

	void calculateNext();

	void linkToMFCCs(int coeffLow, int coeffHigh);
	//void linkToMelBandEnergies(MFCCs* mfcc);
	//void linkToMelSpectrogram(MFCCs* mfcc);

	SelfSimilarityMatrix matrix;

private:
	Master _futureMaster;
	MFCCs _futureMFCCs;

	int _samplesAhead;
};

