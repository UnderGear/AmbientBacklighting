// AmbientBackLightingTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AmbientBackLighting.h"
#include <windows.h>

int main()
{
	//Sleep(10000);

	Config Config;
	Config.HorizontalLightCount = 25;
	Config.VerticalLightCount = 11;

	auto BackLighting = AmbientBackLighting{Config};

	const auto RefreshTime = 1000.f / Config.SamplesPerSecond;

	while (1)
	{
		BackLighting.Update(RefreshTime);
		Sleep(RefreshTime);
	}

    return 0;
}

