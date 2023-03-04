#include "FutureSimilarityMatrix.h"
#include "AudioManager.h"
#include "SignalProcessingManager.h"

FutureSimilarityMatrix::FutureSimilarityMatrix(int size)
	:
matrix(size, SignalProcessingManager::GENERAL_HISTORY_SIZE)
{
}

void FutureSimilarityMatrix::init(int samplesAhead)
{
	_samplesAhead = samplesAhead;

	_futureMaster.init(AudioManager::getAudioData(), AudioManager::getSampleRate());

	_futureMFCCs.init(&_futureMaster, 25, 0, 20000);
}

void FutureSimilarityMatrix::calculateNext()
{
	if (AudioManager::getCurrentSample() + _samplesAhead > AudioManager::getAudioDataSize()) {
		Vengine::warning("Cannot calculate future as samples ahead puts it above song size");
		return;
	}

	_futureMaster.beginCalculations(AudioManager::getCurrentSample() + _samplesAhead);
	_futureMaster.calculateFourierTransform();

	_futureMFCCs.calculateNext();
	matrix.calculateNext();

	_futureMaster.endCalculations();
}

void FutureSimilarityMatrix::linkToMFCCs(int coeffLow, int coeffHigh)
{
	matrix.linkToMFCCs(&_futureMFCCs, coeffLow, coeffHigh);
}
