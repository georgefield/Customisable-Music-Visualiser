#pragma once
#include <vector>

#include "Master.h"
#include "FourierTransform.h"
#include "FFTWapi.h"
#include "FilterBank.h"

class MFCCs
{
public:
	MFCCs() :
		_bandEnergies(nullptr),
		_melSpectrogram(nullptr),
		_mfccs(nullptr),

		_sampleLastCalculated(-1)
	{
	}

	~MFCCs() {
		if (_useSetters) {
			deleteSetters();
		}

		delete[] _bandEnergies;
		delete[] _melSpectrogram;
	}

	void init(Master* m, int numMelBands, float lowerHz, float upperHz, bool useSetters = true) {
		_m = m;

		_filterBank.init(_m);

		createMelLinearlySpacedFilters(numMelBands, lowerHz, upperHz);

		_dct.init(numMelBands);

		//init arrays
		_bandEnergies = new float[numMelBands];
		_melSpectrogram = new float[numMelBands];

		memset(_bandEnergies, 0.0f, numMelBands * sizeof(float));
		memset(_melSpectrogram, 0.0f, numMelBands * sizeof(float));

		//mfccs gets given pointer to output from fftwapi so needs no allocated memory
		_mfccs = _dct.getOutput();

		_useSetters = useSetters;
		if (_useSetters) {
			initSetters();
		}
	}

	void reInit() {
		memset(_bandEnergies, 0.0f, _filterBank.numBands() * sizeof(float));
		memset(_melSpectrogram, 0.0f, _filterBank.numBands() * sizeof(float));
	}

	void calculateNext();


	void debug() {
		for (int i = 0; i < _filterBank.numBands(); i++) {
			std::cout << _filterBank.getBand(i)->lower << " to " << _filterBank.getBand(i)->upper << std::endl;
		}
	}

	float* getBandEnergies() { return _bandEnergies; }

	float* getMelSpectrogram() { return _melSpectrogram; }

	float* getMfccs() { return _mfccs; }

	int getNumMelBands() { return _filterBank.numBands(); }

private:
	Master* _m;
	FFTWdct _dct;

	bool _useSetters;

	int _sampleLastCalculated;

	float mel(float hz);
	float melInverse(float mel);

	void createMelLinearlySpacedFilters(int numFilters, float lowerHz, float upperHz);
	void calculateBandEnergy();
	void calculateMelSpectrogram();
	void calculateMfccs();

	FilterBank _filterBank;
	float* _bandEnergies;
	float* _melSpectrogram;
	float* _mfccs;

	void initSetters();
	void deleteSetters();
};
