#pragma once
#include "SelfSimilarityMatrix.h"
#include "FFTWapi.h"

class SimilarityMatrixHandler
{
public:

	SimilarityMatrixHandler(bool isForNoteOnset = false);
	~SimilarityMatrixHandler();

	void init(Master* master);
	void reInit();

	void calculateNext();

	void relinkBasedOnSPvars();

	void linkToMFCCs(int coeffLow, int coeffHigh);
	void linkToMelBandEnergies();
	void linkToMelSpectrogram();
	void linkToFourierTransform(float cutoffLow = -1.0f, float cutoffHigh = -1.0f, float smoothFactor = 0.0f);

	SelfSimilarityMatrix matrix;

	//getters
	bool isRealTime() { return _samplesAheadForFutureMatrix == 0; }
	LinkedTo isLinkedTo() { return _SMinfo._linkedTo; }

	SimMatInfo _SMinfo;
private:

	Master* _master;
	Master _futureMaster;
	MFCCs _futureMFCCs;

	FourierTransform* _fourierTransform;

	int _samplesAheadForFutureMatrix;
	bool _isForNoteOnset;

	int _counterForDownscale;

	void initUpdaters();
	void removeUpdaters();
};

