#pragma once
#include <math.h>
#include <Vengine/MyErrors.h>
#include <cmath>

enum Kernel {
	LINEAR_PYRAMID,
	GAUSSIAN_PYRAMID,
	TENSION2_PYRAMID,
	TENSIONP5_PYRAMID,
	FLAT
};

class Kernels {
public:
	//can speed up if program needs
	static float apply(Kernel kernel, int k, int n) {
		float scale = 1.0f;
		switch (kernel) {
		case LINEAR_PYRAMID:
			scale = (2.0f / float(n));
			return linear(k, n) * scale;
			break;
		case GAUSSIAN_PYRAMID://no scaling as gaussian integral = 1
			return gaussian(k, n);
		case FLAT:
			return 1.0f / float(n);
		case TENSION2_PYRAMID:
			scale = getTensionScale(2.0f, n);
			return tension(k, n, 2) * scale;
		case TENSIONP5_PYRAMID:
			scale = getTensionScale(0.5f, n);
			return tension(k, n, 0.5) * scale;
		}
	}
private:

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

};