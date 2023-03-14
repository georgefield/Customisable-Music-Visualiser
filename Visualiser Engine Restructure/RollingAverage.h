#pragma once
#include "History.h"

class RollingAverage {
public:
	RollingAverage(int size) :
		history(size),
		sum(0)
	{}

	void add(float value) {
		sum += value;
		if (history.full()) {
			sum -= history.oldest();
		}
		history.add(value);
	}

	float get() {
		if (history.entries() == 0) {
			return 0.0f;
		}
		return sum / float(history.entries());
	}

	void clear() {
		history.clear();
		sum = 0;
	}
private:
	float sum;
	History<float> history;
};