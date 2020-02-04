// AmbientBackLightingTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AmbientBackLighting.h"
#include <windows.h>

int main()
{
	Config Config;

	auto BackLighting = AmbientBackLighting{Config};

	const auto RefreshMilliseconds = 1000.f / Config.SamplesPerSecond;

	auto IsRunning = true;
	while (IsRunning)
	{
		BackLighting.Update(RefreshMilliseconds);
		Sleep(RefreshMilliseconds);

		//TODO: listen for stop input.
	}

    return 0;
}

