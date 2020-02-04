#include "stdafx.h"
#include "AmbientLightStrip.h"
#include <cmath>
#include <algorithm>
#include <vector>

/*const unsigned char AmbientLightStrip::LUT[] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
	2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
	5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
	10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
	17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
	25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
	37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
	51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
	69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
	90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
	115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
	144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
	177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
	215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255
};*/

//TODO: how are we going to initialize the sample color seeds?
AmbientLightStrip::AmbientLightStrip(
	hid_device* InDevice, unsigned int InLightCount, size_t InBufferSize, ScreenSampleInfo InSampleInfo)
	: Device(InDevice), LightCount(InLightCount), BufferSize(InBufferSize), SampleInfo(InSampleInfo)
{
	Spacing = SampleInfo.IsVertical ? SampleInfo.SampleHeight / LightCount : SampleInfo.SampleWidth / LightCount;

	//TODO: report ID, maybe channel should come from the device config.
	ColorBuffer = new unsigned char[BufferSize];
	constexpr unsigned char ReportId = 8;
	ColorBuffer[0] = ReportId;
	constexpr unsigned char Channel = 0;
	ColorBuffer[1] = Channel;

	Pixels = new RGBQUAD[SampleInfo.SampleWidth * SampleInfo.SampleHeight];
	BMI = { 0 };
	BMI.bmiHeader.biSize = sizeof(BMI.bmiHeader);
	BMI.bmiHeader.biWidth = SampleInfo.SampleWidth;
	BMI.bmiHeader.biHeight = SampleInfo.SampleHeight;
	BMI.bmiHeader.biPlanes = 1;
	BMI.bmiHeader.biBitCount = 32;
	BMI.bmiHeader.biCompression = BI_RGB;

	/*std::vector<Color> Seeds
	{
		{0, 255, 255}, //aqua
		{255, 0, 0}, //red
		{0, 255, 0}, //green
		{0, 0, 255}, //blue
		{255, 255, 255}, //white
		{255, 0, 255}, //purple
		{255, 255, 0}, //yellow
		{0, 0, 0}, //black
	};
	ImageSummarizer = new VoronoiImageSummarizer{ Seeds };*/

	auto UseRootMeanSquare = true;
	ImageSummarizer = new AverageImageSummarizer{ UseRootMeanSquare };
}

AmbientLightStrip::~AmbientLightStrip()
{
	delete[] Pixels;

	ClearBuffer();
	hid_send_feature_report(Device, ColorBuffer, BufferSize);

	delete[] ColorBuffer;
	hid_close(Device);

	delete ImageSummarizer;
}

unsigned int GammaCorrect(float Gamma, unsigned int Index)
{
	constexpr unsigned int Min = 0;
	constexpr unsigned int Max = 255;

	//corrected value is (Index/255)^Gamma * Max + 0.5
	auto Value = pow((float)Index / Max, Gamma) * Max + 0.5f;

	//clamp to the range [0, 255]
	Value = max(min(Value, Max), Min);

	//round to the nearest integer
	return (unsigned int)ceil(Value);
}

void AmbientLightStrip::Update(float DeltaTime, HWND& Window)
{
	auto WindowDC = GetWindowDC(Window);
	auto CaptureDC = CreateCompatibleDC(WindowDC);
	auto CaptureBitmap = CreateCompatibleBitmap(WindowDC, SampleInfo.SampleWidth, SampleInfo.SampleHeight);

	SelectObject(CaptureDC, CaptureBitmap);
	BitBlt(CaptureDC, 0, 0, SampleInfo.SampleWidth, SampleInfo.SampleHeight, WindowDC, SampleInfo.SampleOffsetX, SampleInfo.SampleOffsetY, SRCCOPY | CAPTUREBLT);
	GetDIBits(CaptureDC, CaptureBitmap, 0, SampleInfo.SampleHeight, Pixels, &BMI, DIB_RGB_COLORS);

	ReleaseDC(Window, WindowDC);
	DeleteDC(CaptureDC);
	DeleteObject(CaptureBitmap);

	ClearBuffer();

	//TODO: this should probably be extracted to its own series of methods
	for (unsigned int i = 0; i < LightCount; ++i)
	{
		ImageSummarizer->ClearSamples();

		auto SampleStartX = SampleInfo.IsVertical ? 0 : Spacing * i;
		auto SampleEndX = SampleInfo.IsVertical ? SampleInfo.SampleWidth : SampleStartX + Spacing;
		auto SampleStartY = SampleInfo.IsVertical ? Spacing * i : 0;
		auto SampleEndY = SampleInfo.IsVertical ? SampleStartY + Spacing : SampleInfo.SampleHeight;
		unsigned int SampleCount = 0;

		for (unsigned int x = SampleStartX; x <= SampleEndX; ++x)
		{
			for (unsigned int y = SampleStartY; y <= SampleEndY; ++y)
			{
				ImageSummarizer->AddSample({
					Pixels[x + y * SampleInfo.SampleWidth].rgbRed,
					Pixels[x + y * SampleInfo.SampleWidth].rgbGreen,
					Pixels[x + y * SampleInfo.SampleWidth].rgbBlue
				});
			}
		}

		auto Color = ImageSummarizer->GetColor();
		auto r = (unsigned char)Color.R;
		auto g = (unsigned char)Color.G;
		auto b = (unsigned char)Color.B;

		//TODO: look up RGB->HCL, lerp, HCL->RGB. is this actually viable? some mappings just don't work. it's not 1:1
		
		/*ColorBuffer[i * 3 + 2] = LUT[g];
		ColorBuffer[i * 3 + 3] = LUT[r];
		ColorBuffer[i * 3 + 4] = LUT[b];*/
		
		//TODO: figure out some good settings, generate per-channel LUTs based on config gamma values
		//TODO: compare perf on 3 LUTs vs calculation. cache misses might be a big deal.

		//TODO: these gamma values should be read from config, which should be passed in during ctor

		const unsigned int Stride = 3;
		const unsigned int GreenIndexOffset = 2;
		auto GreenGamma = 2.95f;
		ColorBuffer[i * Stride + GreenIndexOffset] = //g;
													 GammaCorrect(GreenGamma, g);

		auto RedGamma = 2.8f;
		const unsigned int RedIndexOffset = 3;
		ColorBuffer[i * Stride + RedIndexOffset] = //r;
												   GammaCorrect(RedGamma, r);

		auto BlueGamma = 3.2f;
		const unsigned int BlueIndexOffset = 4;
		ColorBuffer[i * Stride + BlueIndexOffset] = //b;
													GammaCorrect(BlueGamma, b);
	}

	hid_send_feature_report(Device, ColorBuffer, BufferSize);
}

void AmbientLightStrip::ClearBuffer()
{
	//TODO: start after the buffer header. not hard-coded 2
	for (unsigned int i = 2; i < BufferSize; ++i)
	{
		ColorBuffer[i] = 0;
	}
}

