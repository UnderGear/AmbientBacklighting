#pragma once

#include "stdafx.h"

struct EXPORT ScreenSampleInfo
{
	bool IsVertical = false;
	int SampleWidth;
	int SampleHeight;
	int SampleOffsetX;
	int SampleOffsetY;
};
