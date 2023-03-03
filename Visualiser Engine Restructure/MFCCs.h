#pragma once
#include <vector>

#include "Master.h"
#include "FourierTransform.h"
#include "FFTWapi.h"

struct Band {
	Band(Master* Master, float LowerHz, float UpperHz, float smoothFrac) :
		_master(Master),
		lower(LowerHz),
		upper(UpperHz),
		smoothFrac(smoothFrac)
	{
		float maxHz = float(_master->_sampleRate) / 2.0f; //nyquist freq (max ft goes up to)
		if (upper > maxHz) {
			Vengine::warning("upper hz larger than nyquist frequency, defaulting to nyquist frequency");
			upper = maxHz;
		}
		if (lower < 0) {
			Vengine::warning("lower hz below 0, defaulting to 0");
			lower = 0;
		}

		harmonicLow = floorf((lower / maxHz) * float(_master->_fftHistory.numHarmonics()));
		harmonicHigh = ceilf((upper / maxHz) * float(_master->_fftHistory.numHarmonics()));
	}
	float upper;
	float lower;
	float smoothFrac;
	int harmonicLow;
	int harmonicHigh;

	float getHarmonicFactor(int harmonic) {
		if (smoothFrac == 0.0f) {
			return 1.0f; //no smoothing
		}

		float distanceFromCutoffFrac = float(std::min(harmonic - harmonicLow + 1, harmonicHigh - harmonic + 1)) / float(harmonicHigh - harmonicLow + 1); //plus one on each side so never 0

		return std::min((2.0f * distanceFromCutoffFrac) / smoothFrac, 1.0f); //1.0f => pyramid band, 0.5f => trapezium with top side half of bottom, 0.1f => trapezium top side 9/10 of bottom
	}

private:
	Master* _master;
};

struct FilterBank {

	void add(Master* master, float cutoffLow, float cutoffHigh, float cutoffSmoothFrac) {
		filters.emplace_back(master, cutoffLow, cutoffHigh, cutoffSmoothFrac);
	}

	int numBands() {
		return filters.size();
	}

	Band* getBandPtr(int index) {
		return &filters.at(index);
	}

private:
	std::vector<Band> filters;
};


class MFCCs
{
public:
	MFCCs() :
		_bandEnergies(nullptr),
		_melSpectrogram(nullptr),
		_mfccs(nullptr)
	{
	}

	~MFCCs() {
		deleteSetters();

		delete[] _bandEnergies;
		delete[] _melSpectrogram;
	}

	void init(Master* m, int numMelBands, float lowerHz, float upperHz) {
		_m = m;

		createMelLinearlySpacedFilters(numMelBands, lowerHz, upperHz);

		_dct.init(numMelBands);

		//init arrays
		_bandEnergies = new float[numMelBands];
		_melSpectrogram = new float[numMelBands];

		memset(_bandEnergies, 0.0f, numMelBands * sizeof(float));
		memset(_melSpectrogram, 0.0f, numMelBands * sizeof(float));

		//mfccs gets given pointer to output from fftwapi so needs no allocated memory
		_mfccs = nullptr;

		initSetters();
	}

	void reInit() {
		memset(_bandEnergies, 0.0f, _filterBank.numBands() * sizeof(float));
		memset(_melSpectrogram, 0.0f, _filterBank.numBands() * sizeof(float));
	}

	void calculateNext();


	void debug() {
		for (int i = 0; i < _filterBank.numBands(); i++) {
			std::cout << _filterBank.getBandPtr(i)->lower << " to " << _filterBank.getBandPtr(i)->upper << std::endl;
		}
	}

	float* getBandEnergies() { return _bandEnergies; }

	float* getMelSpectrogram() { return _melSpectrogram; }

	float* getMfccs() { return _mfccs; }

	int getNumMelBands() { return _filterBank.numBands(); }

private:
	Master* _m;
	FFTWdct _dct;


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
