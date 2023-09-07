#pragma once
#include "SimilarityMatrix.h"
#include "SimMatrixStructs.h"
#include "FFTWapi.h"
#include "Master.h"
#include "MFCCs.h"
#include "FourierTransform.h"

class SimilarityMatrixHandler
{
public:

	SimilarityMatrixHandler(bool useSetters);
	~SimilarityMatrixHandler();

	void init(Master* master, MFCCs* mfccs);
	void reInit();

	void calculateNext();

	void relinkBasedOnSPvars();

	void linkToMFCCs(int coeffLow, int coeffHigh);
	void linkToMelBandEnergies();
	void linkToMelSpectrogram();
	void linkToFourierTransform(float cutoffLow = -1.0f, float cutoffHigh = -1.0f, float smoothFactor = 0.0f);

	SimMatInfo _SMinfo;
	SimilarityMatrix* _SM;

	//getters
	LinkedTo isLinkedTo() { return _SMinfo._linkedTo; }

private:

	Master* _master;
	MFCCs* _mfccsPtr;

	FourierTransform* _fourierTransform;

	bool _useSetters;

	int _counterForDownscale;

	float similarityMeasureGetter();
	float matrixStartGetter();
	void initUpdaters();
	void removeUpdaters();
};

