module;

#include "hidapi.h"
#include <windows.h>
import AmbientBackLighting.ScreenSampleInfo;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Config;
import std.core;

export module AmbientBackLighting.AmbientLightStrip;

int GammaCorrect(float Gamma, int Index)
{
	constexpr float Min = 0.f;
	constexpr float Max = 255.f;

	//corrected value is (Index/255)^Gamma * Max + 0.5
	auto Value = std::pow(Index / Max, Gamma) * Max + 0.5f;

	//clamp to the range [0, 255]
	Value = std::clamp(Value, Min, Max);

	//round to the nearest integer
	return (int)Value;
}

export namespace ABL
{
	class AmbientLightStrip
	{
	public:
		AmbientLightStrip(
			hid_device* InDevice, int InLightCount, size_t InBufferSize, ABL::ScreenSampleInfo InSampleInfo)
			: Device(InDevice), LightCount(InLightCount), BufferSize(InBufferSize), SampleInfo(InSampleInfo)
		{
			Spacing = SampleInfo.IsVertical ? SampleInfo.SampleHeight / LightCount : SampleInfo.SampleWidth / LightCount;

			//TODO: report ID, maybe channel should come from the device config.
			ColorBuffer = new unsigned char[BufferSize];
			constexpr unsigned char ReportId = 8;
			ColorBuffer[0] = ReportId;
			constexpr unsigned char Channel = 0;
			ColorBuffer[1] = Channel;

			ScreenSample = new RGBQUAD[SampleInfo.SampleWidth * SampleInfo.SampleHeight];
			BMI = { 0 };
			BMI.bmiHeader.biSize = sizeof(BMI.bmiHeader);
			BMI.bmiHeader.biWidth = SampleInfo.SampleWidth;
			BMI.bmiHeader.biHeight = SampleInfo.SampleHeight;
			BMI.bmiHeader.biPlanes = 1;
			BMI.bmiHeader.biBitCount = 32;
			BMI.bmiHeader.biCompression = BI_RGB;
		}

		~AmbientLightStrip()
		{
			delete[] ScreenSample;

			//send one last message to clear out the LED strip
			ClearBuffer();
			hid_send_feature_report(Device, ColorBuffer, BufferSize);

			delete[] ColorBuffer;
			hid_close(Device);
		}

		void Update(HWND& Window, IImageSummarizer& ImageSummarizer, const ABL::Config& Config)
		{
			//take a snip of the screen for this strip
			UpdateScreenSample(Window);

			//calculate the summarized color for each light
			for (auto i = 0; i < LightCount; ++i)
			{
				UpdateLightAtIndex(i, ImageSummarizer, Config);
			}

			//tell the LED strip what color its lights should be
			hid_send_feature_report(Device, ColorBuffer, BufferSize);
		}
	protected:

		hid_device* Device;
		int LightCount;
		int Spacing;

		size_t BufferSize;
		unsigned char* ColorBuffer;

		ScreenSampleInfo SampleInfo;
		RGBQUAD* ScreenSample;
		BITMAPINFO BMI;

		void UpdateScreenSample(HWND& Window)
		{
			auto WindowDC = GetWindowDC(Window);
			auto CaptureDC = CreateCompatibleDC(WindowDC);
			auto CaptureBitmap = CreateCompatibleBitmap(WindowDC, SampleInfo.SampleWidth, SampleInfo.SampleHeight);

			SelectObject(CaptureDC, CaptureBitmap);
			BitBlt(CaptureDC, 0, 0, SampleInfo.SampleWidth, SampleInfo.SampleHeight, WindowDC, SampleInfo.SampleOffsetX, SampleInfo.SampleOffsetY, SRCCOPY | CAPTUREBLT);
			GetDIBits(CaptureDC, CaptureBitmap, 0, SampleInfo.SampleHeight, ScreenSample, &BMI, DIB_RGB_COLORS);

			ReleaseDC(Window, WindowDC);
			DeleteDC(CaptureDC);
			DeleteObject(CaptureBitmap);
		}

		void UpdateLightAtIndex(int Index, ABL::IImageSummarizer& ImageSummarizer, const ABL::Config& Config)
		{
			//TODO: I think all of these calculations should be handled before we even get to this point.
			//TODO: it should all be pre-calculated during construction
			const auto SampleStartX = SampleInfo.IsVertical ? 0 : Spacing * Index;
			const auto SampleEndX = SampleInfo.IsVertical ? SampleInfo.SampleWidth : SampleStartX + Spacing;
			const auto SampleStartY = SampleInfo.IsVertical ? Spacing * Index : 0;
			const auto SampleEndY = SampleInfo.IsVertical ? SampleStartY + Spacing : SampleInfo.SampleHeight;

			for (int x = SampleStartX; x <= SampleEndX; ++x)
			{
				for (int y = SampleStartY; y <= SampleEndY; ++y)
				{
					const auto SampleIndex = x + y * SampleInfo.SampleWidth;
					const auto& Sample = ScreenSample[SampleIndex];
					ImageSummarizer.AddSample({ Sample.rgbRed, Sample.rgbGreen, Sample.rgbBlue });
				}
			}

			const auto Color = ImageSummarizer.GetColor();

			constexpr int Stride = 3;
			constexpr int GreenIndexOffset = 2;
			ColorBuffer[Index * Stride + GreenIndexOffset] = GammaCorrect(Config.GammaG, (int)Color.G);

			constexpr int RedIndexOffset = 3;
			ColorBuffer[Index * Stride + RedIndexOffset] = GammaCorrect(Config.GammaR, (int)Color.R);

			constexpr int BlueIndexOffset = 4;
			ColorBuffer[Index * Stride + BlueIndexOffset] = GammaCorrect(Config.GammaB, (int)Color.B);
		}

		void ClearBuffer()
		{
			//TODO: update this so it skips the first 2 indices, then we don't need to go back and set 0 and 1 later
			memset(ColorBuffer, 0, BufferSize * sizeof(unsigned char));

			constexpr unsigned char ReportId = 8;
			ColorBuffer[0] = ReportId;
			constexpr unsigned char Channel = 0;
			ColorBuffer[1] = Channel;
		}
	};
}
