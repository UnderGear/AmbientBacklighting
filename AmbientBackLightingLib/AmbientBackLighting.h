#pragma once

#include "Config.h"
#include "hidapi.h"
#include "WindowData.h"

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

	int nScreenWidth;
	int nScreenHeight;
	unsigned int verticalSpacing;
	unsigned int horizontalSpacing;

	size_t BufferSize;

	AmbientLightStrip* LightStripTop = nullptr;
	AmbientLightStrip* LightStripLeft = nullptr;
	AmbientLightStrip* LightStripRight = nullptr;
};