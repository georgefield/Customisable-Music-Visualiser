#include "MFCCs.h"
#include <math.h>
#include "Tools.h"

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
			melInverse(Tools::lerp(mel(lowerHz), mel(upperHz), fractionLow)), 
			melInverse(Tools::lerp(mel(lowerHz), mel(upperHz), fractionHigh)), 1.0f
		);
	}
}
