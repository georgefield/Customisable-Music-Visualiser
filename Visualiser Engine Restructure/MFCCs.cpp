#include "MFCCs.h"
#include <math.h>
#include "MyMaths.h"

void MFCCs::calculateNext()
{
	if (_sampleLastCalculated == _m->_currentSample) {
		return;
	}
	_sampleLastCalculated = _m->_currentSample;

	calculateBandEnergy();
	calculateMelSpectrogram();
	calculateMfccs();
}

void MFCCs::calculateBandEnergy()
{
	memset(_bandEnergies, 0.0f, _filterBank.numBands() * sizeof(float));

	_filterBank.updateAll();

	for (int i = 0; i < _filterBank.numBands(); i++) {
		_bandEnergies[i] = _filterBank.getBand(i)->getBandEnergy();
	}
}

void MFCCs::calculateMelSpectrogram()
{
	memset(_melSpectrogram, 0.0f, _filterBank.numBands() * sizeof(float));

	for (int i = 0; i < _filterBank.numBands(); i++) {
		_melSpectrogram[i] += logf(_bandEnergies[i]);
	}
}

void MFCCs::calculateMfccs()
{
	_dct.calculate(_melSpectrogram);
}


float MFCCs::mel(float hz)
{
	return 1125.0f * logf(1.0f + (hz / 700.0f));
}

float MFCCs::melInverse(float mel)
{
	return 700.0f * (expf(mel / 1125.0f) - 1);
}

void MFCCs::createMelLinearlySpacedFilters(int numFilters, float lowerHz, float upperHz)
{
	float fractionLow = 0;
	float fractionHigh = 0;
	float fractionIncrement = 1.0f / float(numFilters + 1); //+1 as filter they take up an extra increment space
	for (int i = 0; i < numFilters; i++) {
		fractionLow = fractionIncrement * i;
		fractionHigh = fractionLow + (2*fractionIncrement);

		_filterBank.add(
			melInverse(MyMaths::lerp(mel(lowerHz), mel(upperHz), fractionLow)), 
			melInverse(MyMaths::lerp(mel(lowerHz), mel(upperHz), fractionHigh)), 1.0f
		);
	}
}


void MFCCs::initUpdaters()
{
	//mel band energys
	std::function<int()> numMelBandsUpdaterFunction = std::bind(&MFCCs::getNumMelBands, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_numMelBands", numMelBandsUpdaterFunction);

	std::function<float* ()> melBandEnergiesUpdaterFunction = std::bind(&MFCCs::getBandEnergies, this);
	VisualiserShaderManager::SSBOs::setSSBOupdater("vis_melBandEnergies", melBandEnergiesUpdaterFunction, _filterBank.numBands());

	std::function<float* ()> melSpectrogramUpdaterFunction = std::bind(&MFCCs::getMelSpectrogram, this);
	VisualiserShaderManager::SSBOs::setSSBOupdater("vis_melSpectrogram", melSpectrogramUpdaterFunction, _filterBank.numBands());

	std::function<float* ()> mfccUpdaterFunction = std::bind(&MFCCs::getMfccs, this);
	VisualiserShaderManager::SSBOs::setSSBOupdater("vis_MFCCs", mfccUpdaterFunction, _filterBank.numBands());
}

void MFCCs::removeUpdaters()
{
	VisualiserShaderManager::Uniforms::removeUniformUpdater("vis_numMelBands");
	VisualiserShaderManager::SSBOs::removeSSBOupdater("vis_melBandEnergies");
	VisualiserShaderManager::SSBOs::removeSSBOupdater("vis_melSpectrogram");
	VisualiserShaderManager::SSBOs::removeSSBOupdater("vis_MFCCs");
}
