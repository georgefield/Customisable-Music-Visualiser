#include "MFCCs.h"
#include <math.h>
#include "Tools.h"


void MFCCs::calculateNext()
{
	calculateBandEnergy();
	calculateMelSpectrogram();
	calculateMfccs();
}

void MFCCs::calculateBandEnergy()
{
	memset(_bandEnergies, 0.0f, _filterBank.numBands() * sizeof(float));

	for (int i = 0; i < _filterBank.numBands(); i++) {
		for (int j = _filterBank.getBandPtr(i)->harmonicLow; j <= _filterBank.getBandPtr(i)->harmonicHigh; j++) {
			_bandEnergies[i] += powf(_m->_fftHistory.newest()[j] * _filterBank.getBandPtr(i)->getHarmonicFactor(j), 2); //energy is squared
		}
		_bandEnergies[i] /= float(_m->_fftHistory.numHarmonics()); //energy per sample, num harmonics = window size / 2, would divide by window size if working with audio data but because we delete half of fourier transform we divide by num harmonics
		_bandEnergies[i] *= float(_m->_sampleRate); //energy per second
		//although this not accurate anyway as we applied gain to fourier transform but its okay as only calculate from fourier transform. Bother doing this to sync between fourier values and energy
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
	_mfccs = _dct.getOutput();
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

		_filterBank.add(_m,
			melInverse(Tools::lerp(mel(lowerHz), mel(upperHz), fractionLow)), 
			melInverse(Tools::lerp(mel(lowerHz), mel(upperHz), fractionHigh)), 1.0f
		);
	}
}


void MFCCs::initSetters()
{
	//mel band energys
	std::function<int()> numMelBandsSetterFunction = std::bind(&FilterBank::numBands, _filterBank);
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("# Mel bands", numMelBandsSetterFunction);

	std::function<float* ()> melBandEnergiesSetterFunction = std::bind(&MFCCs::getBandEnergies, this);
	VisualiserShaderManager::SSBOs::addPossibleSSBOSetter("Mel band energy", melBandEnergiesSetterFunction, _filterBank.numBands());

	std::function<float* ()> melSpectrogramSetterFunction = std::bind(&MFCCs::getMelSpectrogram, this);
	VisualiserShaderManager::SSBOs::addPossibleSSBOSetter("Mel spectrogram", melSpectrogramSetterFunction, _filterBank.numBands());

	std::function<float* ()> mfccSetterFunction = std::bind(&MFCCs::getMfccs, this);
	VisualiserShaderManager::SSBOs::addPossibleSSBOSetter("MFCCs", mfccSetterFunction, _filterBank.numBands());
}

void MFCCs::deleteSetters()
{
	VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("# Mel bands");
	VisualiserShaderManager::SSBOs::deleteSSBOsetter("Mel band energy");
	VisualiserShaderManager::SSBOs::deleteSSBOsetter("Mel spectrogram");
	VisualiserShaderManager::SSBOs::deleteSSBOsetter("MFCCs");

}
