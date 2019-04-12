#pragma once

#include "stdafx.h"
#include <wchar.h>

struct EXPORT LightStripInfo
{
	unsigned short vendor_id;
	unsigned short product_id;
	const wchar_t* serial;
	size_t buffer_size; //TODO: TBH we should figure out a better way of doing this.
	//TODO: also figure out how the buffer's report id and channel should be set.

	unsigned int light_count;
	bool is_vertical;
	bool is_right_aligned;
	bool is_bottom_aligned;
};

struct EXPORT Config
{
	LightStripInfo Lights[3] =
	{
		{0x20a0, 0x41e5, TEXT("BS021580-3.1"), 2 + 3 * 32, 25, false, false, false},
		{0x20a0, 0x41e5, TEXT("BS021630-3.1"), 2 + 3 * 32, 11, true, false, false},
		{0x20a0, 0x41e5, TEXT("BS021581-3.1"), 2 + 3 * 32, 11, true, true, false}
	};

	unsigned int SampleThickness = 30; //width in pixels of our rectangular samples
	float SamplesPerSecond = 120.f;// 60.f;

	float GammaG = 2.8f;
	float GammaR = 2.8f;
	float GammaB = 2.8f;
};