#pragma once
#include <vector>
#include <map>
#include <chrono>

namespace Vengine {

	class MyTiming
	{
	public:
		static void frameDone();
		static float getFPS();
		static void setNumSamplesForFPS(int samples);
		static int getFrameCount();

		static void startTimer(int& id);
		static float readTimer(int id);
	private:
		static float getTicks();

		static const std::chrono::time_point<std::chrono::steady_clock> _constTimePoint;

		static int _numSamples;
		static std::vector<float> _frameTimings;
		static int _frameCount;

		static std::map<int, long long> _timerStartTicks; //used to store start timer time
	};

}