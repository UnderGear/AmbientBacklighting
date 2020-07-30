// AmbientBackLightingTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AmbientBackLighting.h"
#include <windows.h>

int main()
{
	auto BackLighting = AmbientBackLighting{};

	auto IsRunning = true;
	while (IsRunning)
	{
		BackLighting.Update();
		Sleep(BackLighting.GetRefreshMilliseconds());

		//TODO: listen for stop input.
	}

    return 0;
}

