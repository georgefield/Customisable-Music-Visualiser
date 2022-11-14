#include "MyTiming.h"
#include <utility>


using namespace Vengine;

const std::chrono::time_point<std::chrono::steady_clock> MyTiming::_constTimePoint = std::chrono::steady_clock::now();

///fps stuff------------
int MyTiming::_numSamples = 10;
std::vector<float> MyTiming::_frameTimings(_numSamples);
int MyTiming::_frameCount = 0;

void MyTiming::frameDone() {
	_frameTimings.at(_frameCount % _numSamples) = MyTiming::getTicks();
	_frameCount++;
}

float MyTiming::getFPS() {

	int count = (_numSamples > _frameCount ? _frameCount : _numSamples); //min, if not enough frames yet rendered dont use entire vec

	float fps = 0;
	for (int i = 1; i < count; i++) {
		float diff = _frameTimings.at((_frameCount + i) % _numSamples) - _frameTimings.at((_frameCount + i - 1) % _numSamples);
		fps += 1000000000.0f / (diff * (count - 1)); //1 billion as in nano seconds, (count - 1 as for 10 samples there are 9 differences to average)
		//1000,000,000/avg of difference between frame times of last 10 frames,
	}
	return fps;
}

void MyTiming::setNumSamplesForFPS(int samples) {
	_numSamples = samples;
	_frameTimings.clear();
	_frameTimings.resize(_numSamples);
	_frameCount = 0;
}

int MyTiming::getFrameCount() { return _frameCount; }

///timer stuff--------------
std::map<int, long long> MyTiming::_timerStartTicks;

void MyTiming::startTimer(int& id) {
	
	//timers work by recording the start time and then are later calculated by checking the current time - start time
	auto mit = _timerStartTicks.find(id);
	if (mit == _timerStartTicks.end()) { //texture not loaded yet
		id = _timerStartTicks.size();
		_timerStartTicks.insert(std::make_pair(id, MyTiming::getTicks()));
	}
	else {
		_timerStartTicks[id] = MyTiming::getTicks();
	}
}

float MyTiming::readTimer(int id) { //returns seconds elapsed

	return (MyTiming::getTicks() - _timerStartTicks[id]) * 0.000000001f;
}



//helper function

float MyTiming::getTicks() { //in nanoseconds

	return (std::chrono::steady_clock::now() - _constTimePoint).count();
}