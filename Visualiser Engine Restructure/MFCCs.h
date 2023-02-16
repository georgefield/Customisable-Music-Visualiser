#pragma once
#include <vector>

#include "Master.h"
#include "FourierTransform.h"


struct FilterBank {
	void add(float cutoffLow, float cutoffHigh, float cutoffSmoothFrac) {
		filters.emplace_back(2, cutoffLow, cutoffHigh, cutoffSmoothFrac);
	}

	FourierTransform* get(int index) {
		return &filters.at(index);
	}

	std::vector<FourierTransform> filters;
};


class MFCCs
{
public:
	MFCCs()
	{
	}

	void init(Master* m, int numFilters, float lowerHz, float upperHz) {
		_m = m;

		createMelLinearlySpacedFilters(numFilters, lowerHz, upperHz);

		for (int i = 0; i < _filterBank.filters.size(); i++) {
			_filterBank.filters.at(i).init(_m);
		}
	}

	void debug() {
		for (int i = 0; i < _filterBank.filters.size(); i++) {
			std::cout << _filterBank.filters.at(i).getCutoffLow() << " to " << _filterBank.filters.at(i).getCutoffHigh() << std::endl;
		}
	}

private:
	Master* _m;

	float mel(float hz);
	float melInverse(float mel);

	void createMelLinearlySpacedFilters(int numFilters, float lowerHz, float upperHz);

	FilterBank _filterBank;
};
