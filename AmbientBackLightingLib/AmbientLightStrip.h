#pragma once

#include "hidapi.h"
#include "ScreenSampleInfo.h"


class EXPORT AmbientLightStrip
{
public:
	AmbientLightStrip(hid_device* InDevice, unsigned int InLightCount, size_t InBufferSize, ScreenSampleInfo InSampleInfo);
	~AmbientLightStrip();

	void Update(float DeltaTime, HWND& Window);
	static const unsigned char LUT[256];
protected:

	hid_device* Device;
	unsigned int LightCount;
	unsigned int Spacing;

	size_t BufferSize;
	unsigned char* ColorBuffer;

	ScreenSampleInfo SampleInfo;
	RGBQUAD* Pixels;
	BITMAPINFO BMI;

	void ClearBuffer();
};