#pragma once
#include <set>
#include <Vengine/MyErrors.h>

#include "History.h"
#include "Peak.h"

struct ValueSamplePair {
	float value;
	int sample;
};

class Threshold {
public:
	Threshold(int numValuesToAverageOver) :
		_valuesAddedInTimeOrder(numValuesToAverageOver),
		aboveThreshold(false)
	{}

	void addValue(float value, int sample = -1) {
		_sortedValues.insert(value);
		if (_valuesAddedInTimeOrder.full()) {
			removeOneOfValue(_valuesAddedInTimeOrder.oldest().value);
		}
		_valuesAddedInTimeOrder.add({ value, sample });
	}

	bool testThreshold(float value, float topXpercent) {
		//1/2 of threshold full then start returning peaks
		if ((value * std::min((2.0f * getFractionFull()), 1.0f)) > getTopXpercentValue(topXpercent)) {
			return true;
		}
		return false;
	}

	bool currentlyInPeak() {
		return aboveThreshold;
	}

	bool getLastPeak(float topXpercent, Peak& out) {

		//when first above threshold clear vectors
		if (goneBelowFractionOfThreshold && testThreshold(_valuesAddedInTimeOrder.newest().value, topXpercent)) {
			samplesOfPointsAboveThreshold.clear();
			valuesOfPointsAboveThreshold.clear();
			aboveThreshold = true;
			goneBelowFractionOfThreshold = false;
		}

		if (!testThreshold(_valuesAddedInTimeOrder.newest().value, topXpercent)) {
			aboveThreshold = false;
		}

		if (!testThreshold(_valuesAddedInTimeOrder.newest().value * 1.5, topXpercent)) {
			goneBelowFractionOfThreshold = true;
		}

		//only choose peak onset and salience after value goes to below half the threshold
		if (goneBelowFractionOfThreshold) {
			float integral = 0;
			float weightedSum = 0;
			float max = 0;
			for (int i = 0; i < valuesOfPointsAboveThreshold.size(); i++) {
				max = std::max(max, valuesOfPointsAboveThreshold.at(i));
				integral += valuesOfPointsAboveThreshold.at(i);
				weightedSum += valuesOfPointsAboveThreshold.at(i) * samplesOfPointsAboveThreshold.at(i);
			}
			out.onset = int(weightedSum / integral);
			out.salience = max;

			return true;
		}

		//while above threshold add info the vectors that choose the peak
		if (aboveThreshold) {
			samplesOfPointsAboveThreshold.push_back(_valuesAddedInTimeOrder.newest().sample);
			valuesOfPointsAboveThreshold.push_back(_valuesAddedInTimeOrder.newest().value);
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

	bool goneBelowFractionOfThreshold;
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

	History<ValueSamplePair> _valuesAddedInTimeOrder;

	std::multiset<float> _sortedValues;
};