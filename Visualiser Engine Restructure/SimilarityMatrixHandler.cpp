#include "SimilarityMatrixHandler.h"
#include "AudioManager.h"
#include "SignalProcessingManager.h"
#include "VisVars.h"
#include "UIglobalFeatures.h"

#include <Vengine/MyTiming.h>
#include <assert.h>

SimilarityMatrixHandler::SimilarityMatrixHandler(bool useSetters)
	:
	_master(nullptr),
	_mfccsPtr(nullptr),
	_fourierTransform(nullptr),

	_counterForDownscale(0),
	_useSetters(useSetters)
{
}

SimilarityMatrixHandler::~SimilarityMatrixHandler()
{
	if (_fourierTransform != nullptr) {
		delete _fourierTransform;
	}
	if (_useSetters) {
		removeUpdaters();
	}
}

void SimilarityMatrixHandler::init(Master* master, MFCCs* mfccs)
{
	_master = master;
	_mfccsPtr = mfccs;

	//fourier transform inited when created
	if (_useSetters) {
		initUpdaters();
	}
}

void SimilarityMatrixHandler::reInit()
{
	relinkBasedOnSPvars();

	if (_SMinfo._linkedTo == FT) {
		_fourierTransform->reInit();
	}
}

void SimilarityMatrixHandler::calculateNext()
{
	if (_SMinfo._linkedTo == NONE) return; //dont bother doing anything if not linked

	//calculate only 1 out of every resolution factor
	_counterForDownscale = (_counterForDownscale + 1) % _SMinfo._downscale;
	if (_counterForDownscale != 0) {
		return;
	}

	//add vectors based on linkedTo flag
	switch (_SMinfo._linkedTo)
	{
	case MFCC:
		_SM->add(_mfccsPtr->getMfccs(), _SMinfo._contrastFactor);
		break;
	case MelBandEnergies:
		_SM->add(_mfccsPtr->getBandEnergies(), _SMinfo._contrastFactor);
		break;
	case MelSpectrogram:
		_SM->add(_mfccsPtr->getMelSpectrogram(), _SMinfo._contrastFactor);
		break;
	case FT:
		_fourierTransform->calculateNext();
		_SM->add(_fourierTransform->getOutput(), _SMinfo._contrastFactor);
		break;
	default:
		Vengine::warning("Invalid linkedTo flag for sim. mat.");
		break;
	}
}

void SimilarityMatrixHandler::relinkBasedOnSPvars()
{
	if (_SMinfo._linkedTo == NONE) {
		return;
	}

	if (_SMinfo._linkedTo == MFCC) {
		linkToMFCCs(_SMinfo._coeffLow, _SMinfo._coeffHigh);
		return;
	}
	if (_SMinfo._linkedTo == MelBandEnergies) {
		linkToMelBandEnergies();
		return;
	}
	if (_SMinfo._linkedTo == MelSpectrogram) {
		linkToMelSpectrogram();
		return;
	}
	if (_SMinfo._linkedTo == FT) {
		linkToFourierTransform(_SMinfo._cutoffLow, _SMinfo._cutoffHigh, _SMinfo._cutoffSmoothFactor);
		return;
	}
}

void SimilarityMatrixHandler::linkToMFCCs(int coeffLow, int coeffHigh)
{
	//sanity check
	assert(coeffLow > 0);
	assert(coeffHigh <= Vis::consts._numMelBands);
	assert(coeffLow <= coeffHigh);

	//set SM info
	_SMinfo._coeffLow = coeffLow;
	_SMinfo._coeffHigh = coeffHigh;
	_SMinfo._linkedTo = MFCC;

	//create new SM object for MFCCs
	if (_SM != nullptr) delete _SM; //delete old if it exists
	_SM = new SimilarityMatrix(_SMinfo._matrixSize, _SMinfo._coeffHigh - _SMinfo._coeffLow + 1);
}

void SimilarityMatrixHandler::linkToMelBandEnergies()
{
	//set SM info
	_SMinfo._linkedTo = MelBandEnergies;

	//create new SM object for MFCCs
	if (_SM != nullptr) delete _SM; //delete old if it exists
	_SM = new SimilarityMatrix(_SMinfo._matrixSize, _mfccsPtr->getNumMelBands());
}

void SimilarityMatrixHandler::linkToMelSpectrogram()
{
	//set SM info
	_SMinfo._linkedTo = MelSpectrogram;

	//create new SM object for MFCCs
	if (_SM != nullptr) delete _SM; //delete old if it exists
	_SM = new SimilarityMatrix(_SMinfo._matrixSize, _mfccsPtr->getNumMelBands());
}

void SimilarityMatrixHandler::linkToFourierTransform(float cutoffLow, float cutoffHigh, float smoothFactor)
{
	_SMinfo._cutoffLow = cutoffLow;
	_SMinfo._cutoffHigh = cutoffHigh;
	_SMinfo._cutoffSmoothFactor = smoothFactor;
	_SMinfo._linkedTo = FT;

	//delete old fourier transform if exists
	if (_fourierTransform != nullptr) delete _fourierTransform;
	_fourierTransform = new FourierTransform(1, cutoffLow, cutoffHigh, smoothFactor);
	_fourierTransform->init(_master, -1);

	//create new SM object for FT
	if (_SM != nullptr) delete _SM; //delete old if it exists
	_SM = new SimilarityMatrix(_SMinfo._matrixSize, _fourierTransform->getNumHarmonics());
}


float SimilarityMatrixHandler::similarityMeasureGetter()
{
	if (_SM == nullptr) return 0;
	return _SM->getSimilarityMeasure();
	//return _SM->getSimilarityMeasureThreaded(); threaded only faster for v large matrices because of overhead (> 1000)
}

float SimilarityMatrixHandler::matrixStartGetter()
{
	return 2 * float(_SM->getDataStart() % _SM->matrixSize()) / float(_SM->matrixSize()) - 1;
}

void SimilarityMatrixHandler::initUpdaters()
{
	std::function<float()> similarityMeasureUpdaterFunction = std::bind(&SimilarityMatrixHandler::similarityMeasureGetter, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_similarityMeasure", similarityMeasureUpdaterFunction);
}

void SimilarityMatrixHandler::removeUpdaters()
{
	VisualiserShaderManager::Uniforms::removeUniformUpdater("vis_similarityMeasure");
}
