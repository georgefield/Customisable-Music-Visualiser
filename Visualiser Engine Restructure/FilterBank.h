#pragma once
struct Band {
	Band(FourierTransform* masterFT, float LowerHz, float UpperHz, float smoothFrac) :
		_masterFT(masterFT),
		lower(LowerHz),
		upper(UpperHz),
		smoothFrac(smoothFrac),
		_bandEnergy(0),
		_previousBandEnergy(0),
		_2prevBandEnergy(0),
		harmonicHigh(0),
		harmonicLow(0)
	{
	}
	float upper;
	float lower;
	float smoothFrac;
	int harmonicLow;
	int harmonicHigh;

	void init() {
		float maxHz = _masterFT->getMasterPtr()->nyquist();
		if (upper > maxHz) {
			Vengine::warning("upper hz larger than nyquist frequency, defaulting to nyquist frequency");
			upper = maxHz;
		}
		if (lower < 0) {
			Vengine::warning("lower hz below 0, defaulting to 0");
			lower = 0;
		}

		harmonicLow = floorf((lower / maxHz) * float(_masterFT->getMasterPtr()->_fftHistory.numHarmonics()));
		harmonicHigh = ceilf((upper / maxHz) * float(_masterFT->getMasterPtr()->_fftHistory.numHarmonics()));

		if (_masterFT->_FTinfo.cutoffLow != 0 || _masterFT->_FTinfo.cutoffHigh != _masterFT->getMasterPtr()->nyquist()) {
			Vengine::fatalError("Only pass full spectrum FT as master to band");
		}
	}

	void reInit() {
		init();
	}

	void calculateBandEnergy() {

		_2prevBandEnergy = _previousBandEnergy;
		_previousBandEnergy = _bandEnergy;
		_bandEnergy = 0;
		for (int j = harmonicLow; j < harmonicHigh; j++) {
			_bandEnergy += powf(_masterFT->getOutput()[j] * getHarmonicFactor(j), 2); //energy is squared
		}
		_bandEnergy /= float(_masterFT->getNumHarmonics()); //energy per sample, num harmonics = window size / 2, would divide by window size if working with audio data but because we delete half of fourier transform we divide by num harmonics
		_bandEnergy *= float(_masterFT->getMasterPtr()->_sampleRate); //energy per second
		//although this not accurate anyway as we applied gain to fourier transform but its okay as only calculate from fourier transform. Bother doing this to sync between fourier values and energy
	}

	float getBandEnergy() { return _bandEnergy; }
	float getPrevBandEnergy() { return _previousBandEnergy; }
	float getPrevPrevBandEnergy() { return _2prevBandEnergy; }

private:
	FourierTransform* _masterFT;
	float _bandEnergy;
	float _previousBandEnergy;
	float _2prevBandEnergy;

	float getHarmonicFactor(int harmonic) {
		if (smoothFrac == 0.0f) {
			return 1.0f; //no smoothing
		}

		float distanceFromCutoffFrac = float(std::min(harmonic - harmonicLow + 1, harmonicHigh - harmonic + 1)) / float(harmonicHigh - harmonicLow + 1); //plus one on each side so never 0

		return std::min((2.0f * distanceFromCutoffFrac) / smoothFrac, 1.0f); //1.0f => pyramid band, 0.5f => trapezium with top side half of bottom, 0.1f => trapezium top side 9/10 of bottom
	}
};

struct FilterBank {

	FilterBank() :
		masterTransform(1)
	{
	}

	void init(Master* master) {
		masterTransform.init(master, -1);
	}

	void reInit() {
		masterTransform.reInit();
		for (auto& it : filters) {
			it.reInit();
		}
	}

	void add(float cutoffLow, float cutoffHigh, float cutoffSmoothFrac) {
		filters.emplace_back(&masterTransform, cutoffLow, cutoffHigh, cutoffSmoothFrac);
		filters.back().init();
	}

	int numBands() {
		return filters.size();
	}

	void updateAll(bool customFT = false) {

		if (!customFT) {
			masterTransform.calculateNext();
		}

		for (auto& it : filters) {
			it.calculateBandEnergy();
		}
	}

	Band* getBand(int index) {
		return &filters.at(index);
	}

	FourierTransform* getMasterTransform() { return &masterTransform; }

private:
	FourierTransform masterTransform;
	std::vector<Band> filters;
};