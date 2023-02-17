#include "SelfSimilarityMatrix.h"

void SimilarityMatrix::calculateNext()
{
	if (_linkedTo == NONE) {
		Vengine::warning("Self similarity matrix not linked to any vector data, no calculation performed");
		return;
	}

	if (_linkedTo == MFCC) {
		_similarityMatrix.add(_mfccsPtr->getBandEnergy())
	}
}

void SimilarityMatrix::linkToMFCCs(MFCCs* mfcc)
{
	_mfccsPtr = mfcc;
	_linkedTo = MFCC;
}

void SimilarityMatrix::linkToMelBandEnergies(MFCCs* mfcc)
{
	_mfccsPtr = mfcc;
	_linkedTo = MelBandEnergies;
}

void SimilarityMatrix::linkToMelSpectrogram(MFCCs* mfcc)
{
	_mfccsPtr = mfcc;
	_linkedTo = MelSpectrogram;
}

void SimilarityMatrix::linkToFourierTransform(FourierTransform* ft)
{
	_ftPtr = ft;
	_linkedTo = FT;
}
