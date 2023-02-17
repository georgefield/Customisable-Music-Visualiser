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
	_bandEnergy.clear();
	_bandEnergy.resize(_filterBank.numBands());

	for (int i = 0; i < _bandEnergy.size(); i++) {
		for (int j = _filterBank.getBandPtr(i)->harmonicLow; j <= _filterBank.getBandPtr(i)->harmonicHigh; j++) {
			_bandEnergy[i] += powf(_m->_fftHistory.newest()[j] * _filterBank.getBandPtr(i)->getHarmonicFactor(j), 2); //energy is squared
		}
	}
}

void MFCCs::calculateMelSpectrogram()
{
	_melSpectrogram.clear();
	_melSpectrogram.resize(_filterBank.numBands());

	for (int i = 0; i < _filterBank.numBands(); i++) {
		_melSpectrogram[i] += logf(_bandEnergy[i]);
	}
}

void MFCCs::calculateMfccs()
{
	_dct.calculate(&(_melSpectrogram[0]));
	float* out = _dct.getOutput();
	_mfccs.clear();
	for (int i = 0; i < _dct.windowSize(); i++) {
		_mfccs.push_back(out[i]);
	}
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
