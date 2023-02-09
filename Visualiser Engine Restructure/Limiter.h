#pragma once
#include <string>
#include <Vengine/MyTiming.h>
#include <math.h>

#include "Threshold.h"
#include "Tools.h"

class Limiter {
public:
	//attack & release in seconds
	Limiter(float ceiling, float attack, float release, float medianValueTarget = -1, float numEntriesToAverageMedianOver = 500) :
		_ceiling(ceiling),
		_attack(attack),
		_release(release),
		_medianValueTarget(medianValueTarget),

		_C(1.0f),
		_timeOfLastHigh(0),
		_previousHigh(0),
		_thresholder(numEntriesToAverageMedianOver),
		_lastCallTime(0),
		_currentGainAdjustment(1.0f)
	{
		Vengine::MyTiming::startTimer(_timerID);
	}

	float clampMedianThenLimit(float value) {
		return limitValue(clampMedianValue(value));
	}

	float clampMedianValue(float value) { //increase or decrease gain to set the median value to median target value
		if (_medianValueTarget == -1) {
			Vengine::warning("Clamp median value called but median value target not set");
			return value;
		}

		//add value to thresholder (required to find median value)
		_thresholder.addValue(value);

		float medianValue = _thresholder.getTopXpercentValue(50); //50 percent (median)
		if (medianValue == -1) { //not enough entries
			return 1.0f; //no adjustment
		}

		float factorToHitMedian = (_medianValueTarget / medianValue);

		if (factorToHitMedian > MAX_GAIN_FACTOR) {
			factorToHitMedian = MAX_GAIN_FACTOR; //clamp gain to MAX_GAIN_FACTOR to stop any craziness
		}

		_currentGainAdjustment = Tools::lerp(1.0f, factorToHitMedian, _thresholder.getFractionFull()); //only use full power when thresholder data full

		return value * _currentGainAdjustment;
	} 

	float limitValue(float value, float gain = 1) {

		value *= gain;

		float _timeSinceLastHigh = Vengine::MyTiming::readTimer(_timerID) - _timeOfLastHigh;

		float desiredC = _ceiling / _previousHigh; //desiredC the C value that would cause the previous high = ceiling
		//take _C to desiredC in attack time
		if (_previousHigh > _ceiling && _timeSinceLastHigh < _attack) {
			_C = Tools::lerp(_C, desiredC, _timeSinceLastHigh / _attack);
		}
		//start to release after attack period
		else if (_timeSinceLastHigh > _attack) {

			//_C goes back from desiredC to 1
			_C = Tools::lerp(desiredC, 1, ((_timeSinceLastHigh - _attack) / (_release * desiredC)));

			if (_C > 1) { _C = 1; } //clamp to 1 as only decrease gain
		}

		float limitedValue = value * _C;

		//new high if value > previous high OR the limited value is above the ceiling after the full attack has been completed for the last high
		if (value > _previousHigh || (_timeSinceLastHigh > _attack && limitedValue > _ceiling)) {
			_previousHigh = value;
			_timeOfLastHigh = Vengine::MyTiming::readTimer(_timerID);
		}

		return limitedValue;
	}

private:

	const float MAX_GAIN_FACTOR = 100.0f;

	int _timerID;
	float _timeOfLastHigh;

	//the factor to adjust value down by (part of limiter, between 0 & 1)
	float _C;

	float _ceiling, _attack, _release; //attack & release in seconds
	float _previousHigh;

	//needed only for when adjusting gain automatically
	float _medianValueTarget; //adjust gain automatically to set middle value to this
	Threshold _thresholder; //only use if medianValueTarget set
	float _lastCallTime;
	float _currentGainAdjustment;
};