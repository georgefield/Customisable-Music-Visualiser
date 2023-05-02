#pragma once

class MyMaths
{
public:

	static float lerp(float c1, float c2, float t) {
		return ((1 - t) * c1) + (t * c2);
	}

	static float dot(float* v1, float* v2, int dim) {
		float ret = 0;
		for (int i = 0; i < dim; i++) {
			ret += v1[i] * v2[i];
		}
		return ret;
	}
	
	static float L2norm(float* v1, int dim) {
		float ret = 0;
		for (int i = 0; i < dim; i++) {
			ret += v1[i] * v1[i];
		}
		return sqrtf(ret);
	}

	static float L1distanceMetric(float* v1, float* v2, int dim) {

		float ret = 0;
		for (int i = 0; i < dim; i++) {
			ret += fabsf(v1[i] - v2[i]);
		}
		return ret;
	}

	static float L1distanceMetricIncDimOnly(float* v1, float* v2, int dim) {

		float ret = 0;
		for (int i = 0; i < dim; i++) {
			if (v2[i] - v1[i] > 0)
				ret += v2[i] - v1[i];
		}
		return ret;
	}

	static float L2distanceMetric(float* v1, float* v2, int dim) {

		float ret = 0;
		for (int i = 0; i < dim; i++) {
			ret += (v1[i] - v2[i]) * (v1[i] - v2[i]);
		}
		return sqrtf(ret);
	}

	static float L2distanceMetricIncDimOnly(float* v1, float* v2, int dim) {

		float ret = 0;
		for (int i = 0; i < dim; i++) {
			if (v2[i] - v1[i] > 0)
				ret += (v1[i] - v2[i]) * (v1[i] - v2[i]);
		}
		return sqrt(ret);
	}
};