module;
#include "hidapi.h"
#include <windows.h>

export module AmbientBackLighting.AmbientLightStrip;
import AmbientBackLighting.ScreenSampleInfo;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Config;
import std.core;

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
			hid_device* InDevice, std::size_t InLightCount, std::size_t InBufferSize, ABL::ScreenSampleInfo InSampleInfo)
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
			BitmapInfo = { 0 };
			BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
			BitmapInfo.bmiHeader.biWidth = SampleInfo.SampleWidth;
			BitmapInfo.bmiHeader.biHeight = SampleInfo.SampleHeight;
			BitmapInfo.bmiHeader.biPlanes = 1;
			BitmapInfo.bmiHeader.biBitCount = 32;
			BitmapInfo.bmiHeader.biCompression = BI_RGB;
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
			for (std::size_t i = 0; i < LightCount; ++i)
			{
				UpdateLightAtIndex(i, ImageSummarizer, Config);
			}

			//tell the LED strip what color its lights should be
			hid_send_feature_report(Device, ColorBuffer, BufferSize);
		}
	protected:

		hid_device* Device;
		std::size_t LightCount;
		std::size_t Spacing;

		std::size_t BufferSize;
		unsigned char* ColorBuffer;

		ScreenSampleInfo SampleInfo;
		RGBQUAD* ScreenSample;
		BITMAPINFO BitmapInfo;

		void UpdateScreenSample(HWND& Window)
		{
			auto WindowDC = GetWindowDC(Window);
			auto CaptureDC = CreateCompatibleDC(WindowDC);
			auto CaptureBitmap = CreateCompatibleBitmap(WindowDC, SampleInfo.SampleWidth, SampleInfo.SampleHeight);

			SelectObject(CaptureDC, CaptureBitmap);
			BitBlt(CaptureDC, 0, 0, SampleInfo.SampleWidth, SampleInfo.SampleHeight, WindowDC, SampleInfo.SampleOffsetX, SampleInfo.SampleOffsetY, SRCCOPY | CAPTUREBLT);
			GetDIBits(CaptureDC, CaptureBitmap, 0, SampleInfo.SampleHeight, ScreenSample, &BitmapInfo, DIB_RGB_COLORS);

			ReleaseDC(Window, WindowDC);
			DeleteDC(CaptureDC);
			DeleteObject(CaptureBitmap);
		}

		void UpdateLightAtIndex(std::size_t LightIndex, ABL::IImageSummarizer& ImageSummarizer, const ABL::Config& Config)
		{
			//TODO: I think all of these calculations should be handled before we even get to this point.
			//TODO: it should all be pre-calculated during construction
			const auto SampleStartX = SampleInfo.IsVertical ? 0 : Spacing * LightIndex;
			const auto SampleEndX = SampleInfo.IsVertical ? SampleInfo.SampleWidth : SampleStartX + Spacing;
			const auto SampleStartY = SampleInfo.IsVertical ? Spacing * LightIndex : 0;
			const auto SampleEndY = SampleInfo.IsVertical ? SampleStartY + Spacing : SampleInfo.SampleHeight;

			for (auto x = SampleStartX; x <= SampleEndX; ++x)
			{
				for (auto y = SampleStartY; y <= SampleEndY; ++y)
				{
					const auto SampleIndex = x + y * SampleInfo.SampleWidth;
					const auto& Sample = ScreenSample[SampleIndex];
					ImageSummarizer.AddSample({ Sample.rgbRed, Sample.rgbGreen, Sample.rgbBlue }); //TODO: this is expensive. we're calling the ctor here.
				}
			}

			const auto Color = ImageSummarizer.GetColor();
			ImageSummarizer.ClearSamples();

			//TODO: we really have to kill off these conversions and use the right types. the hardware expects unsigned chars.
			// that's basically 1 byte per color channel.
			constexpr std::size_t Stride = 3;
			constexpr std::size_t GreenIndexOffset = 2; // this is after the buffer header.
			ColorBuffer[LightIndex * Stride + GreenIndexOffset] = GammaCorrect(Config.GammaG, (int)Color.G); //TODO: again, gross c-style casts

			constexpr std::size_t RedIndexOffset = 3;
			ColorBuffer[LightIndex * Stride + RedIndexOffset] = GammaCorrect(Config.GammaR, (int)Color.R);

			constexpr std::size_t BlueIndexOffset = 4;
			ColorBuffer[LightIndex * Stride + BlueIndexOffset] = GammaCorrect(Config.GammaB, (int)Color.B);
		}

		void ClearBuffer()
		{
			// clear the buffer except for the report id and channel info at the front
			constexpr std::size_t BufferHeaderSize = 2;
			memset(ColorBuffer, BufferHeaderSize, BufferSize * sizeof(unsigned char) - BufferHeaderSize);
		}
	};
}
