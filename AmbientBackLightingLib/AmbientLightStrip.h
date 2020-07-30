#pragma once

#include "hidapi.h"
#include "ScreenSampleInfo.h"
#include "ImageSummarizer.h"

class EXPORT AmbientLightStrip
{
public:
	AmbientLightStrip(hid_device* InDevice, int InLightCount, size_t InBufferSize, ScreenSampleInfo InSampleInfo);
	~AmbientLightStrip();

	void Update(HWND& Window, IImageSummarizer& ImageSummarizer, const struct Config& Config);
protected:

	hid_device* Device;
	int LightCount;
	int Spacing;

	size_t BufferSize;
	unsigned char* ColorBuffer;

	ScreenSampleInfo SampleInfo;
	RGBQUAD* ScreenSample;
	BITMAPINFO BMI;

	void UpdateScreenSample(HWND& Window);
	void UpdateLightAtIndex(int Index, IImageSummarizer& ImageSummarizer, const struct Config& Config);

	void ClearBuffer();
};
