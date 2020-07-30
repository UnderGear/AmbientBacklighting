#pragma once

#include "Config.h"
#include "hidapi.h"
#include "WindowData.h"
#include <vector>

class EXPORT AmbientBackLighting
{
public:
	void Update();
	AmbientBackLighting();
	~AmbientBackLighting();

	int GetRefreshMilliseconds() const { return (int)(1000.f / AppConfig.SamplesPerSecond); }

protected:

	Config AppConfig;
	WindowSelector WindowSelector;
	class IImageSummarizer* ImageSummarizer;
	std::vector<class AmbientLightStrip*> LightStrips;
};