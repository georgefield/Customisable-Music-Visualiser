#pragma once
#include <set>
#include <Vengine/MyErrors.h>

#include "History.h"


class Threshold {
public:
	Threshold(int numValuesToAverageOver) :
		_valuesAddedInTimeOrder(numValuesToAverageOver)
	{}

	void addValue(float value) {
		_sortedValues.insert(value);
		if (_valuesAddedInTimeOrder.full()) {
			removeOneOfValue(_valuesAddedInTimeOrder.oldest());
		}
		_valuesAddedInTimeOrder.add(value);
	}

	bool testThreshold(float value, float topXpercent) {
		if (value > getTopXpercentValue(10)) {
			return true;
		}
		return false;
	}

	float getTopXpercentValue(float topXpercent) {

		auto it = _sortedValues.end();
		std::advance(it, int(-ceilf(topXpercent * 0.01 * _sortedValues.size()))); //move backwards from end topXpercent percent
		if (it == _sortedValues.end()) { //avoid any errors caused by set being too small
			return -1.0f;
		}
		return *it;
	}

	float getFractionFull() {
		return float(getNumValues()) / float(getTotalSize());
	}

	int getTotalSize() {
		return _valuesAddedInTimeOrder.totalSize();
	}

	int getNumValues() {
		return _valuesAddedInTimeOrder.entries();
	}

private:
	void removeOneOfValue(float value) { //done automatically so user does not need access
		auto it = _sortedValues.find(value);
		if (it != _sortedValues.end()) {
			_sortedValues.erase(it);
		}
		else {
			Vengine::warning("Could not find the value to erase");
		}
	}

	History<float> _valuesAddedInTimeOrder;

	std::multiset<float> _sortedValues;
};