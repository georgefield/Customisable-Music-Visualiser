#pragma once
#include <set>
#include <Vengine/MyErrors.h>

#include "History.h"
#include "Peak.h"

class Threshold {
public:
	Threshold(int numValuesToAverageOver) :
		_valuesAddedInTimeOrder(numValuesToAverageOver),
		aboveThreshold(false)
	{}

	void addValue(float value, int sample = -1) {
		_sortedValues.insert(value);
		if (_valuesAddedInTimeOrder.full()) {
			removeOneOfValue(_valuesAddedInTimeOrder.oldest());
		}
		_valuesAddedInTimeOrder.add(value, sample);
	}

	bool testThreshold(float value, float topXpercent) {
		if ((value * getFractionFull()) > getTopXpercentValue(topXpercent)) {
			return true;
		}
		return false;
	}

	bool getLastPeak(float topXpercent, Peak& out) {

		if (testThreshold(_valuesAddedInTimeOrder.newest(), topXpercent)) {
			if (!aboveThreshold) {
				samplesOfPointsAboveThreshold.clear();
				valuesOfPointsAboveThreshold.clear();
				aboveThreshold = true;
			}
			samplesOfPointsAboveThreshold.push_back(_valuesAddedInTimeOrder.newestSample());
			valuesOfPointsAboveThreshold.push_back(_valuesAddedInTimeOrder.newest());
		}
		else if (aboveThreshold == true) {
			float integral = 0;
			float weightedSum = 0;
			for (int i = 0; i < valuesOfPointsAboveThreshold.size(); i++) {
				integral += valuesOfPointsAboveThreshold.at(i);
				weightedSum += valuesOfPointsAboveThreshold.at(i) * samplesOfPointsAboveThreshold.at(i);
			}
			out.onset = int(weightedSum / integral);
			out.salience = integral / valuesOfPointsAboveThreshold.size();

			aboveThreshold = false;
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

	bool aboveThreshold;
	std::vector<int> samplesOfPointsAboveThreshold;
	std::vector<float> valuesOfPointsAboveThreshold;
	int lastPeakSample;

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