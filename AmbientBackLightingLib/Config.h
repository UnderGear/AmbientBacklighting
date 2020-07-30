#pragma once

#include "stdafx.h"
#include <wchar.h>

enum EXPORT LightStripAlignment
{
	Top = 0,
	Left,
	Right,
	Bottom
};

struct EXPORT LightStripInfo
{
	unsigned short vendor_id;
	unsigned short product_id;
	const wchar_t* serial;
	size_t buffer_size; //TODO: we should be able to derive this...
	//TODO: also figure out how the buffer's report id and channel should be set.

	unsigned int light_count;

	LightStripAlignment alignment;
};

struct EXPORT Config
{
	//TODO: we have GOT to get rid of this random math in here. it's more or less the size of the buffer?
	LightStripInfo Lights[3] =
	{
		{0x20a0, 0x41e5, TEXT("BS021580-3.1"), 2 + 3 * 32, 25, Top},
		{0x20a0, 0x41e5, TEXT("BS021630-3.1"), 2 + 3 * 32, 11, Left},
		{0x20a0, 0x41e5, TEXT("BS021581-3.1"), 2 + 3 * 32, 11, Right}
	};

	int SampleThickness = 30; //width in pixels of our rectangular samples
	int SamplesPerSecond = 120;// 60;

	//old gamma values from main game playthrough: (G: 2.4, R: 1.9, B: 3.4)
	//initial B&W playthrough gamma values: (G: 1.9, R: 1.5, B: 2.7)

	float GammaG = 2.05f;
	float GammaR = 1.8f;
	float GammaB = 2.5f;
};