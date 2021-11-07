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
	class AmbientLightStrip
	{
		// Physical device for the light strip
		hid_device* Device;
		// Buffer for writing to the device
		unsigned char* ColorBuffer;

		// Light config data
		const ABL::LightStripInfo& LightInfo;
		// Sample data for this light strip
		ABL::ScreenSampleInfo SampleInfo;
		// Sample data per light in this strip
		std::vector<ABL::LightSampleInfo> Lights;

		// Screen sample
		RGBQUAD* ScreenSample;
		BITMAPINFO BitmapInfo;

	public:
		AmbientLightStrip(
			hid_device* InDevice, const ABL::LightStripInfo& InLightInfo, std::size_t ScreenWidth, std::size_t ScreenHeight, std::size_t SampleThickness)
			: Device(InDevice), LightInfo(InLightInfo)
		{
			const auto IsVertical = LightInfo.Alignment == ABL::LightStripAlignment::Left || LightInfo.Alignment == ABL::LightStripAlignment::Right;
			SampleInfo = ABL::ScreenSampleInfo
			{
				// is vertical, width, height, offset x, offset y
				IsVertical,
				IsVertical ? SampleThickness : ScreenWidth,
				IsVertical ? ScreenHeight : SampleThickness,
				LightInfo.Alignment == ABL::LightStripAlignment::Right ? ScreenWidth - SampleThickness : 0,
				LightInfo.Alignment == ABL::LightStripAlignment::Bottom ? ScreenHeight - SampleThickness : 0
			};

			const auto Spacing = SampleInfo.IsVertical
				? static_cast<float>(SampleInfo.SampleHeight) / LightInfo.LightCount
				: static_cast<float>(SampleInfo.SampleWidth) / LightInfo.LightCount;

			Lights.reserve(LightInfo.LightCount);
			for (std::size_t LightIndex = 0; LightIndex < LightInfo.LightCount; ++LightIndex)
			{
				Lights.emplace_back(LightIndex, SampleInfo, Spacing);
			}

			ColorBuffer = new unsigned char[LightInfo.BufferSize];
			ColorBuffer[0] = LightInfo.ReportId;
			ColorBuffer[1] = LightInfo.Channel;

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
			hid_send_feature_report(Device, ColorBuffer, LightInfo.BufferSize);

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
			hid_send_feature_report(Device, ColorBuffer, LightInfo.BufferSize);
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

		void UpdateLight(ABL::IImageSummarizer& ImageSummarizer, const ABL::Config& Config, const ABL::LightSampleInfo& Light)
		{
			// add the color of every pixel in our screen sample associated with this light to our image summarizer
			for (auto x = Light.SampleStartX; x <= Light.SampleEndX; ++x)
			{
				for (auto y = Light.SampleStartY; y <= Light.SampleEndY; ++y)
				{
					const auto SampleIndex = x + y * SampleInfo.SampleWidth; // 2D -> 1D array index conversion
					const auto& SamplePixel = ScreenSample[SampleIndex];
					ImageSummarizer.AddSample({ SamplePixel.rgbRed, SamplePixel.rgbGreen, SamplePixel.rgbBlue });
					//TODO: this is expensive. we're calling the ctor here for a color.
					//TODO: we're also having to use the vtable to make this call. gross. I guess we could use CRTP if we really wanted?
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
			memset(ColorBuffer, BufferHeaderSize, LightInfo.BufferSize * BufferItemSize - BufferHeaderSize);
		}
	};
}
