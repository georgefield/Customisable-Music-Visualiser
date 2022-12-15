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
	static float apply(Kernel kernel, int k, int n) {
		switch (kernel) {
		case LINEAR_PYRAMID:
			return linear(k, n);
			break;
		case GAUSSIAN_PYRAMID:
			return gaussian(k, n);
		case FLAT:
			return 1; //because all weighted equally
		case TENSION2_PYRAMID:
			return tension(k, n, 1);
		case TENSIONP5_PYRAMID:
			return tension(k, n, 0.5);
		}
	}
private:

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