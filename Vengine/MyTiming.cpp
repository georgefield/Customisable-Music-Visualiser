#include "MyTiming.h"
#include <utility>
#include <SDL/SDL.h>

#include "MyErrors.h"

#include <iostream>
#include <cassert>


using namespace Vengine;

const std::chrono::time_point<std::chrono::steady_clock> MyTiming::_epochTimePoint = std::chrono::steady_clock::now();

///fps stuff------------
float MyTiming::_minDeltaT = 0; //no limit
float MyTiming::_deltaT = 1.0; //set to non zero to avoid errors
int MyTiming::_numFPSsamples = 10;
std::vector<long long> MyTiming::_frameTimings(_numFPSsamples);
int MyTiming::_frameCount = 0;
std::map<int, MyTiming::WhenRead> MyTiming::_whenTimerRead;



void MyTiming::frameDone() {
	if (_frameCount != 0) { //delay next frame if completed frame faster than fps limit
		_deltaT = ticksToSeconds(MyTiming::ticksSinceEpoch() - _frameTimings.at((_frameCount) % _numFPSsamples));
		if (_deltaT < _minDeltaT) {
			SDL_Delay(Uint32((_minDeltaT - _deltaT) * 1000));
		}
	}

	updateWhenRead();
	_frameCount++;
	_frameTimings.at(_frameCount % _numFPSsamples) = MyTiming::ticksSinceEpoch();
}


float MyTiming::getFPS() {

	int count = (_numFPSsamples > _frameCount ? _frameCount : _numFPSsamples); //min, if not enough frames yet rendered dont use entire vec

	float fps = 0;
	float prevDeltaT = ticksToSeconds(_frameTimings.at((_frameCount) % _numFPSsamples) - _frameTimings.at((_frameCount + 1) % _numFPSsamples));
	//frame count + 1 oldest, frame count newest
	fps += 1.0f / prevDeltaT * (count - 1); //(count - 1 as for 10 samples there are 9 differences to average)
	//1/total of difference between frame times of last [_numFPSsamples] frames, = fps
	return fps;
}

void MyTiming::setNumSamplesForFPS(int samples) {
	_numFPSsamples = samples;
	_frameTimings.clear();
	_frameTimings.resize(_numFPSsamples);
	_frameCount = 0;
}

int MyTiming::getFrameCount() { return _frameCount; }

void MyTiming::setFPSlimit(unsigned int limit) {

	_minDeltaT = 1.0f / limit;
}


///timer stuff--------------
std::map<int, long long> MyTiming::_timerStartTicks;
std::map<int, long long> MyTiming::_timerPauseTicks;

void MyTiming::createTimer(int& id) {

	//id generation
	id = _timerStartTicks.size();
	while (_timerStartTicks.find(id) != _timerStartTicks.end()) {
		id++;
	}

	//timers work by recording the start time and then are later calculated by checking the current time - start time
	_timerStartTicks[id] = MyTiming::ticksSinceEpoch();
	_timerPauseTicks[id] = MyTiming::ticksSinceEpoch(); //always start paused

	_whenTimerRead[id] = { false, false }; //0 for no
}

void MyTiming::eraseTimer(int id) {
	assert(_timerStartTicks.find(id) != _timerStartTicks.end());

	_timerStartTicks.erase(id);
	_whenTimerRead.erase(id);

	if (_timerPauseTicks.find(id) != _timerPauseTicks.end()) { //if paused then return time when paused by taking away time since paused
		_timerPauseTicks.erase(id);
	}
}

float MyTiming::readTimer(int id) { //returns seconds elapsed

	assert(_timerStartTicks.find(id) != _timerStartTicks.end());

	_whenTimerRead[id].readThisFrame = true;

	long long ticks;

	if (_timerPauseTicks.find(id) != _timerPauseTicks.end()) { //if paused then return time when paused by taking away time since paused
		ticks = _timerPauseTicks[id] - _timerStartTicks[id];
	}
	else {
		ticks = MyTiming::ticksSinceEpoch() - _timerStartTicks[id];
	}

	return ticksToSeconds(ticks);
}

void Vengine::MyTiming::startTimer(int id) {

	assert(_timerStartTicks.find(id) != _timerStartTicks.end());

	if (_timerPauseTicks.find(id) == _timerPauseTicks.end()) {
		return; //timer already going
	}

	long long timePaused = (MyTiming::ticksSinceEpoch() - _timerPauseTicks[id]);
	_timerStartTicks[id] += timePaused;

	_timerPauseTicks.erase(id);
}

void Vengine::MyTiming::stopTimer(int id)
{
	assert(_timerStartTicks.find(id) != _timerStartTicks.end());

	if (_timerPauseTicks.find(id) != _timerPauseTicks.end()) {
		return; //timer already stopped
	}
	_timerPauseTicks[id] = MyTiming::ticksSinceEpoch();
}

void Vengine::MyTiming::resetTimer(int id)
{
	assert(_timerStartTicks.find(id) != _timerStartTicks.end());

	_timerStartTicks[id] = MyTiming::ticksSinceEpoch();
	_timerPauseTicks[id] = MyTiming::ticksSinceEpoch();
}

void Vengine::MyTiming::setTimer(int id, float time)
{
	assert(_timerStartTicks.find(id) != _timerStartTicks.end());

	_timerStartTicks[id] = MyTiming::ticksSinceEpoch() - secondsToTicks(time);
	_timerPauseTicks[id] = MyTiming::ticksSinceEpoch();
}

bool Vengine::MyTiming::timerReadLastFrame(int id)
{
	return _whenTimerRead[id].readLastFrame;
}

//helper functions

long long MyTiming::ticksSinceEpoch() { //in nanoseconds

	return (std::chrono::steady_clock::now() - _epochTimePoint).count();
}

float MyTiming::ticksToSeconds(long long ticks) {

	return ticks * 0.000000001f;
}

long long MyTiming::secondsToTicks(float seconds) {

	return seconds * 1000000000.0f;
}

void Vengine::MyTiming::updateWhenRead()
{
	for (auto& it : _whenTimerRead) {
		it.second.readLastFrame = it.second.readThisFrame;
		it.second.readThisFrame = false;
	}
}
