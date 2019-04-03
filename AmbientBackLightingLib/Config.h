#pragma once

#include "stdafx.h"

struct EXPORT Config
{
	unsigned int HorizontalLightCount;
	unsigned int VerticalLightCount;
	unsigned int SampleThickness = 60; //width in pixels of our rectangular samples
	float SamplesPerSecond = 120.f;// 60.f;

	float GammaG = 2.8f;
	float GammaR = 2.8f;
	float GammaB = 2.8f;
};