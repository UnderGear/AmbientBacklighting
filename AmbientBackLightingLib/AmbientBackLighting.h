#pragma once

#include "Config.h"
#include "hidapi.h"
#include "WindowData.h"
#include <vector>

class AmbientLightStrip;

class EXPORT AmbientBackLighting
{
public:
	void Update(float DeltaTime);
	AmbientBackLighting(Config InConfig);
	~AmbientBackLighting();

protected:

	Config AppConfig;
	WindowSelector WindowSelector;
	std::vector<AmbientLightStrip*> LightStrips;
};