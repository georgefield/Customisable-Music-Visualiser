#pragma once
#include <string>
#include <Vengine/MyTiming.h>
#include <math.h>

class Limiter {
public:
	//attack & release in seconds
	Limiter(float ceiling, float attack, float release) : _ceiling(ceiling), _attack(attack), _release(release), _C(1.0f), _timeOfLastHigh(0), _previousHigh(0) {
		Vengine::MyTiming::startTimer(_timerID);
	}

	float limitValue(float value, float gain = 1) {

		value *= gain;

		float deltaT = Vengine::MyTiming::readTimer(_timerID) - _timeOfLastHigh;

		float desiredC = _ceiling / _previousHigh;
		if (_previousHigh > _ceiling && deltaT < _attack) {
			_C = lerp(_C, desiredC, deltaT / _attack);
		}
		else if (deltaT > _attack) {

			//do release on log scale as tiny multpilier values cause release to go fast even for really long releases
			//float lnC = logf(_C);
			//_C = expf(lerp(lnC, 0, ((deltaT - _attack) / _release)));

			//release depended on desired C
			_C = lerp(desiredC, 1, ((deltaT - _attack) / (_release * desiredC)));

			if (_C > 1) { _C = 1; } //stop overshoot
		}

		float limitedValue = value * _C;

		if (value > _previousHigh || (deltaT > _attack && limitedValue > _ceiling)) {
			_previousHigh = value;
			_timeOfLastHigh = Vengine::MyTiming::readTimer(_timerID);
		}

		return limitedValue;
	}

private:

	//t ranging from 0 to 1, c1 -> c2
	float lerp(float c1, float c2, float t) {
		return ((1 - t) * c1) + (t * c2);
	}

	int _timerID;
	float _timeOfLastHigh;

	float _C;

	float _ceiling, _attack, _release;
	float _previousHigh;
};