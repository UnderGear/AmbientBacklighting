#pragma once

#include "stdafx.h"

struct EXPORT Config
{
	unsigned int HorizontalLightCount;
	unsigned int VerticalLightCount;
	unsigned int SampleThickness = 60; //width in pixels of our rectangular samples
	float SamplesPerSecond = 120.f;// 60.f;
};