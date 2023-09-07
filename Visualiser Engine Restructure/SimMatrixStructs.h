#pragma once
static enum LinkedTo {
	NONE = 4,
	DEBUG = 5,
	MFCC = 0,
	MelBandEnergies = 1,
	MelSpectrogram = 2,
	FT = 3
};

static enum MeasureType {
	SIMILARITY = 0,
	PERCUSSION = 1
};

struct SimMatInfo {
	LinkedTo _linkedTo = LinkedTo::MFCC;
	int _matrixSize = 80;
	int _coeffLow = 1;
	int _coeffHigh = 10;
	float _cutoffLow = 0;
	float _cutoffHigh = 20000;
	float _cutoffSmoothFactor = 0.0f;
	int _downscale = 1;
	MeasureType _measureType = SIMILARITY;
	float _contrastFactor = 5.0f;

	friend bool operator==(SimMatInfo info1, SimMatInfo info2) {
		bool isEqual = info1._linkedTo == info2._linkedTo &&
			info1._matrixSize == info2._matrixSize &&
			info1._downscale == info2._downscale;

		if (info1._linkedTo == LinkedTo::MFCC) {
			isEqual &= info1._coeffLow == info2._coeffLow &&
				info1._coeffHigh == info2._coeffHigh;
		}
		if (info1._linkedTo == LinkedTo::FT) {
			isEqual &= info1._cutoffLow == info2._cutoffLow &&
				info1._cutoffHigh == info2._cutoffHigh &&
				info1._cutoffSmoothFactor == info2._cutoffSmoothFactor;
		}
		return isEqual;
	}

	friend bool operator!=(SimMatInfo info1, SimMatInfo info2) {
		return !(info1 == info2);
	}
};