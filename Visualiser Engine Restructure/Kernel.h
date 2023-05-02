#pragma once
#include <math.h>
#include <Vengine/MyErrors.h>
#include <cmath>

static enum KernelType {
	LINEAR_PYRAMID,
	GAUSSIAN_PYRAMID,
	TENSION2_PYRAMID,
	TENSIONP5_PYRAMID,
	SAWTOOTH,
	FLAT
};

class Kernel {
public:
	Kernel() : _kernelType(FLAT), _n(1), _scale(1){}

	void setUp(KernelType kernelType, int n) {
		_n = n;
		_scale = 1.0f;
		_kernelType = kernelType;
		switch (_kernelType) {
		case LINEAR_PYRAMID:
			_scale = (2.0f / float(n));
			break;
		case GAUSSIAN_PYRAMID:
		case FLAT:
			break;
		case TENSION2_PYRAMID:
			_scale = getTensionScale(2.0f, _n);
			break;
		case TENSIONP5_PYRAMID:
			_scale = getTensionScale(0.5f, n);
			break;
		case SAWTOOTH:
			_scale = (2.0f / float(n));
			break;
		default:
			Vengine::fatalError("Invalid kernel type submitted");
		}
	}

	float getValueAt(int k) {
		switch (_kernelType) {
		case LINEAR_PYRAMID:
			return linear(k, _n) * _scale;
			break;
		case GAUSSIAN_PYRAMID://no scaling as gaussian integral = 1
			return gaussian(k, _n);
		case FLAT:
			return 1.0f / float(_n);
		case TENSION2_PYRAMID:
			return tension(k, _n, 2) * _scale;
		case TENSIONP5_PYRAMID:
			return tension(k, _n, 0.5) * _scale;
		case SAWTOOTH:
			return sawtooth(k, _n) * _scale;
		}
	}
private:

	int _n;
	float _scale;
	KernelType _kernelType;

	static float getTensionScale(float t, int n) {
		return ((t + 1) / float(n)) * powf(2.0f / float(n), t);
	}

	static float linear(int k, int n) {
		return 1 - ((2.0f / n) * fabsf(k - ((n - 1) / 2)));
	}

	static float tension(int k, int n, float t) {
		return powf(linear(k, n), t);
	}

	static float gaussian(int k, int n) {
		int x = k - ((n - 1) / 2);
		return expf( - (x * x) / (n / 2)); 
	}

	static float sawtooth(int k, int n) {
		return 1 - ((k + 0.5) / n);
	}
};