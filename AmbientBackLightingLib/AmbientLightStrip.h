#pragma once

#include "hidapi.h"
#include "ScreenSampleInfo.h"

class EXPORT AmbientLightStrip
{
public:
	AmbientLightStrip(hid_device* InDevice, unsigned int InLightCount, size_t InBufferSize, ScreenSampleInfo InSampleInfo);
	~AmbientLightStrip();

	void Update(float DeltaTime, HWND& Window);

	//TODO: pull these out somewhere and generate them based on config data.
	static const unsigned char LUTG[256];
	static const unsigned char LUTR[256];
	static const unsigned char LUTB[256];

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
