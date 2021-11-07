module;
#include "hidapi.h"
#include <windows.h>

export module AmbientBackLighting.AmbientLightStrip;
import AmbientBackLighting.ScreenSampleInfo;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Config;
import std.core;

export namespace ABL
{
	struct LightData
	{
		std::size_t LightIndex = 0;
		std::size_t SampleStartX = 0;
		std::size_t SampleEndX = 0;
		std::size_t SampleStartY = 0;
		std::size_t SampleEndY = 0;

		constexpr LightData(std::size_t InLightIndex, const ABL::ScreenSampleInfo& SampleInfo, std::size_t Spacing)
			: LightIndex(InLightIndex)
		{
			SampleStartX = SampleInfo.IsVertical ? 0 : Spacing * LightIndex;
			SampleEndX = SampleInfo.IsVertical ? SampleInfo.SampleWidth : SampleStartX + Spacing;
			SampleStartY = SampleInfo.IsVertical ? Spacing * LightIndex : 0;
			SampleEndY = SampleInfo.IsVertical ? SampleStartY + Spacing : SampleInfo.SampleHeight;
		}
	};

	class AmbientLightStrip
	{
		hid_device* Device;

		std::vector<LightData> Lights;

		std::size_t BufferSize;
		unsigned char* ColorBuffer;

		ABL::ScreenSampleInfo SampleInfo;
		RGBQUAD* ScreenSample;
		BITMAPINFO BitmapInfo;

	public:
		AmbientLightStrip(
			hid_device* InDevice, std::size_t LightCount, std::size_t InBufferSize, ABL::ScreenSampleInfo InSampleInfo)
			: Device(InDevice), BufferSize(InBufferSize), SampleInfo(InSampleInfo)
		{
			const auto Spacing = SampleInfo.IsVertical ? SampleInfo.SampleHeight / LightCount : SampleInfo.SampleWidth / LightCount;

			Lights.reserve(LightCount);
			for (std::size_t LightIndex = 0; LightIndex < LightCount; ++LightIndex)
			{
				Lights.emplace_back(LightIndex, SampleInfo, Spacing);
			}

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

		void Update(HWND& Window, ABL::IImageSummarizer& ImageSummarizer, const ABL::Config& Config)
		{
			//take a snip of the screen for this strip
			UpdateScreenSample(Window);

			//calculate the summarized color for each light
			for (const auto& Light : Lights)
			{
				UpdateLight(ImageSummarizer, Config, Light);
			}

			//tell the LED strip what color its lights should be
			hid_send_feature_report(Device, ColorBuffer, BufferSize);
		}

	private:

		//TODO: compare this approach of sampling only the desired sections of the screen per light strip vs sampling the full screen once for all
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

		void UpdateLight(ABL::IImageSummarizer& ImageSummarizer, const ABL::Config& Config, const LightData& Light)
		{
			//TODO: can we use ranges somehow to eliminate these raw for loops?
			for (auto x = Light.SampleStartX; x <= Light.SampleEndX; ++x)
			{
				for (auto y = Light.SampleStartY; y <= Light.SampleEndY; ++y)
				{
					const auto SampleIndex = x + y * SampleInfo.SampleWidth;
					const auto& Sample = ScreenSample[SampleIndex];
					ImageSummarizer.AddSample({ Sample.rgbRed, Sample.rgbGreen, Sample.rgbBlue }); //TODO: this is expensive. we're calling the ctor here.
				}
			}

			const auto Color = ImageSummarizer.GetColor();
			ImageSummarizer.ClearSamples();

			constexpr std::size_t Stride = 3;
			constexpr std::size_t GreenIndexOffset = 2; // this is after the buffer header's 2 entries.
			ColorBuffer[Light.LightIndex * Stride + GreenIndexOffset] = GammaCorrect(Config.GammaG, Color.G);

			constexpr std::size_t RedIndexOffset = 3;
			ColorBuffer[Light.LightIndex * Stride + RedIndexOffset] = GammaCorrect(Config.GammaR, Color.R);

			constexpr std::size_t BlueIndexOffset = 4;
			ColorBuffer[Light.LightIndex * Stride + BlueIndexOffset] = GammaCorrect(Config.GammaB, Color.B);
		}

		unsigned char GammaCorrect(float Gamma, unsigned long RawValue)
		{
			constexpr float Min = 0.f;
			constexpr float Max = 255.f;

			//corrected value is (Index/255)^Gamma * Max + 0.5
			auto CorrectedValue = std::pow(RawValue / Max, Gamma) * Max + 0.5f;

			//clamp to the range [0, 255]
			CorrectedValue = std::clamp(CorrectedValue, Min, Max);

			//round to the nearest integer
			return static_cast<unsigned char>(CorrectedValue);
		}

		void ClearBuffer()
		{
			// clear the buffer except for the report id and channel info at the front
			constexpr std::size_t BufferHeaderSize = 2;
			constexpr auto BufferItemSize = sizeof(unsigned char);
			memset(ColorBuffer, BufferHeaderSize, BufferSize * BufferItemSize - BufferHeaderSize);
		}
	};
}
