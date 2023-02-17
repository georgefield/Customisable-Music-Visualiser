#include "SelfSimilarityMatrix.h"

static bool ayo = true;

void SelfSimilarityMatrix::calculateNext()
{
	if (_linkedTo == NONE) {
		Vengine::warning("Self similarity matrix not linked to any vector data, no calculation performed");
		return;
	}

	if (_linkedTo == MFCC) {
		_similarityMatrix.add(_mfccsPtr->getMfccs());
		return;
	}
	if (_linkedTo == MelBandEnergies) {
		_similarityMatrix.add(_mfccsPtr->getBandEnergy());
		return;
	}
	if (_linkedTo == MelSpectrogram) {
		_similarityMatrix.add(_mfccsPtr->getMelSpectrogram());
		return;
	}
	if (_linkedTo == FT) {
		_similarityMatrix.add(_ftPtr->getOutput(), _ftPtr->getNumHarmonics());
		return;
	}

	if (_linkedTo == DEBUG) {
		std::vector<float> lol;
		if (ayo) {

			lol.push_back(1);
			lol.push_back(0);
		}
		else {
			lol.push_back(0);
			lol.push_back(1);
		}

		std::cout << Vengine::MyTiming::readTimer(_debugTimerId) << std::endl;
		if (Vengine::MyTiming::readTimer(_debugTimerId) > 0.5) {
			Vengine::MyTiming::stopTimer(_debugTimerId);
			Vengine::MyTiming::startTimer(_debugTimerId);
			ayo = !ayo;
		}

		_similarityMatrix.add(lol);
		return;
	}
}

void SelfSimilarityMatrix::linkToMFCCs(MFCCs* mfcc)
{
	_mfccsPtr = mfcc;
	_linkedTo = MFCC;
}

void SelfSimilarityMatrix::linkToMelBandEnergies(MFCCs* mfcc)
{
	_mfccsPtr = mfcc;
	_linkedTo = MelBandEnergies;
}

void SelfSimilarityMatrix::linkToMelSpectrogram(MFCCs* mfcc)
{
	_mfccsPtr = mfcc;
	_linkedTo = MelSpectrogram;
}

void SelfSimilarityMatrix::linkToFourierTransform(FourierTransform* ft)
{
	_ftPtr = ft;
	_linkedTo = FT;
}

void SelfSimilarityMatrix::linkToDebug()
{
	_linkedTo = DEBUG;
	Vengine::MyTiming::startTimer(_debugTimerId);
}
