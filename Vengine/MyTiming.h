#pragma once
#include <vector>
#include <map>
#include <chrono>

namespace Vengine {

	class MyTiming
	{
	public:
		struct WhenRead {
			bool readThisFrame;
			bool readLastFrame;
		};

		static void frameDone();
		static float getFPS();
		static void setNumSamplesForFPS(int samples);
		static int getFrameCount();
		static void setFPSlimit(unsigned int limit);

		static void startTimer(int& id);
		static float readTimer(int id);
		static bool timerReadLastFrame(int id);
	private:
		static long long ticksSinceEpoch();
		static float ticksToSeconds(long long ticks);

		static const std::chrono::time_point<std::chrono::steady_clock> _epochTimePoint;

		static float _minDeltaT; //calculated from fpslimit input
		static float _deltaT;
		static int _numFPSsamples;
		static std::vector<long long> _frameTimings;
		static int _frameCount;

		static std::map<int, long long> _timerStartTicks; //used to store start timer time

		static void updateWhenRead();
		static std::map<int, WhenRead> _whenTimerRead;
	};

}