#include <windows.h>
import std.core;
import AmbientBackLighting.BackLighting;
import Profiler;

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	auto BackLighting = ABL::AmbientBackLighting{};

	auto IsRunning = true;

	//Profiler::FrameProfiler Profiler;
	while (IsRunning)
	{
		BackLighting.Update();
		//Profiler.Print();
		//Profiler.Reset();

		//TODO: read from config for our refresh rate again.
		// 1 count of a duration that is 1/120 seconds long
		constexpr auto SleepDuration = std::chrono::duration<double, std::ratio<1, 120>> { 1 };
		std::this_thread::sleep_for(SleepDuration);

		//TODO: listen for stop input.
	}

	return 0;
}
