#include "SelfSimilarityMatrix.h"

void SelfSimilarityMatrix::calculateNext()
{
	if (_linkedTo == NONE) {
		Vengine::warning("Self similarity matrix not linked to any vector data, no calculation performed");
		return;
	}

	if (_linkedTo == MFCC) {
		_similarityMatrix.add(_mfccsPtr->getMfccs(), _mfccsPtr->getNumMelBands());
		return;
	}
	if (_linkedTo == MelBandEnergies) {
		_similarityMatrix.add(_mfccsPtr->getBandEnergies(), _mfccsPtr->getNumMelBands());
		return;
	}
	if (_linkedTo == MelSpectrogram) {
		_similarityMatrix.add(_mfccsPtr->getMelSpectrogram(), _mfccsPtr->getNumMelBands());
		return;
	}
	if (_linkedTo == FT) {
		_similarityMatrix.add(_ftPtr->getOutput(), _ftPtr->getNumHarmonics());
		return;
	}

	if (_linkedTo == DEBUG) {
		std::vector<float> debugVec;
		if (_switch) {

			debugVec.push_back(1);
			debugVec.push_back(0);
		}
		else {
			debugVec.push_back(0);
			debugVec.push_back(1);
		}

		std::cout << Vengine::MyTiming::readTimer(_debugTimerId) << std::endl;
		if (Vengine::MyTiming::readTimer(_debugTimerId) > 1.0) {
			Vengine::MyTiming::stopTimer(_debugTimerId);
			Vengine::MyTiming::startTimer(_debugTimerId);
			_switch = !_switch;
		}

		_similarityMatrix.add(debugVec);
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
	_switch = true;
}

void SelfSimilarityMatrix::debug()
{
	int n = std::min(10, _similarityMatrix.sideLength());
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			std::cout << std::fixed << std::setprecision(2) << getSelfSimilarityMatrixValue(i, j) << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-------" << std::endl;
}
