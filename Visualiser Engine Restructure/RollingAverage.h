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
		return sum / float(history.totalSize());
	}
private:
	float sum;
	History<float> history;
};