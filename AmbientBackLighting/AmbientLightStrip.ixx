module;
#include "hidapi.h"
#include <windows.h>

export module AmbientBackLighting.AmbientLightStrip;
import AmbientBackLighting.ScreenSampleInfo;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Config;
import Profiler;
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
		ABL::RGBSampler Sampler = {};
		// Sample data per light in this strip
		std::vector<ABL::LightSampleInfo> Lights;

		// Screen sample
		RGBQUAD* ScreenSample;
		BITMAPINFO BitmapInfo;

		HWND& Window;
		HDC WindowDC;
		HDC CaptureDC;
		HBITMAP CaptureBitmap;

	public:
		AmbientLightStrip(
			HWND& InWindow, hid_device* InDevice, const ABL::LightStripInfo& InLightInfo, std::size_t ScreenWidth, std::size_t ScreenHeight, std::size_t SampleThickness)
			: Device(InDevice), LightInfo(InLightInfo), Window(InWindow)
		{
			const auto IsVertical = LightInfo.Alignment == ABL::LightStripAlignment::Left || LightInfo.Alignment == ABL::LightStripAlignment::Right;
			SampleInfo = ABL::ScreenSampleInfo
			{
				// width, height, offset x, offset y, is vertical
				IsVertical ? SampleThickness : ScreenWidth,
				IsVertical ? ScreenHeight : SampleThickness,
				LightInfo.Alignment == ABL::LightStripAlignment::Right ? ScreenWidth - SampleThickness : 0,
				LightInfo.Alignment == ABL::LightStripAlignment::Bottom ? ScreenHeight - SampleThickness : 0,
				IsVertical
			};

			//TODO: implicit conversions abound
			const auto Spacing = static_cast<std::size_t>(SampleInfo.IsVertical
				? static_cast<float>(SampleInfo.SampleHeight) / static_cast<float>(LightInfo.LightCount)
				: static_cast<float>(SampleInfo.SampleWidth) / static_cast<float>(LightInfo.LightCount));
			const auto Padding = (Spacing - SampleThickness) / 2;

			Lights.reserve(LightInfo.LightCount);
			for (std::size_t LightIndex = 0; LightIndex < LightInfo.LightCount; ++LightIndex)
			{
				Lights.emplace_back(LightIndex, SampleInfo, SampleThickness, Padding);
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

			WindowDC = GetWindowDC(Window);
			CaptureDC = CreateCompatibleDC(WindowDC);
			CaptureBitmap = CreateCompatibleBitmap(WindowDC, SampleInfo.SampleWidth, SampleInfo.SampleHeight);
		}

		~AmbientLightStrip()
		{
			delete[] ScreenSample;

			//send one last message to clear out the LED strip
			ClearBuffer();
			hid_send_feature_report(Device, ColorBuffer, LightInfo.BufferSize);

			delete[] ColorBuffer;
			hid_close(Device);

			ReleaseDC(Window, WindowDC);
			DeleteDC(CaptureDC);
			DeleteObject(CaptureBitmap);
		}

		void Update(const ABL::Config& Config)
		{
			//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::Update", 0 };

			{
				//take a snip of the screen for this strip
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateScreenSample", 1 };
				UpdateScreenSample();
			}

			//calculate the summarized color for each light
			for (const auto& Light : Lights)
			{
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateLight", 2 };
				UpdateLight(Config, Light);
			}

			{
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::Update - Send To LED Strip", 3 };
				//TODO: this is super expensive. we're spending .04s on this for 3 lights every cycle
				//tell the LED strip what color its lights should be
				hid_send_feature_report(Device, ColorBuffer, LightInfo.BufferSize);
			}
		}

	private:

		//TODO: what if we updated screen samples on a per-light basis instead? what if we reduced the size for each light sample, too?
		void UpdateScreenSample()
		{
			SelectObject(CaptureDC, CaptureBitmap);
			BitBlt(CaptureDC, 0, 0, SampleInfo.SampleWidth, SampleInfo.SampleHeight, WindowDC, SampleInfo.SampleOffsetX, SampleInfo.SampleOffsetY, SRCCOPY | CAPTUREBLT);
			GetDIBits(CaptureDC, CaptureBitmap, 0, SampleInfo.SampleHeight, ScreenSample, &BitmapInfo, DIB_RGB_COLORS);
		}

		void UpdateLight(const ABL::Config& Config, const ABL::LightSampleInfo& Light)
		{
			// add the color of every pixel in our screen sample associated with this light to our image summarizer
			for (auto x = Light.SampleStartX; x <= Light.SampleEndX; ++x)
			{
				for (auto y = Light.SampleStartY; y <= Light.SampleEndY; ++y)
				{
					const auto SampleIndex = x + y * SampleInfo.SampleWidth; // 2D -> 1D array index conversion
					const auto& SamplePixel = ScreenSample[SampleIndex];
					//Profiler::StackFrameProfile StackFrame = { "ImageSummarizer::AddSample", 4 };
					Sampler.AddSample(static_cast<double>(SamplePixel.rgbRed), static_cast<double>(SamplePixel.rgbGreen), static_cast<double>(SamplePixel.rgbBlue));
				}
			}

			auto Color = Sampler.GetColor(Config.Gammas);
			Sampler.ClearSamples();

			{
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateLight - Update Buffer", 6 };
				constexpr std::size_t Stride = 3;
				constexpr std::size_t GreenBufferOffset = 2; // this is after the buffer header's 2 entries.
				constexpr std::size_t GreenChannelIndex = 2;
				ColorBuffer[Light.LightIndex * Stride + GreenBufferOffset] = static_cast<unsigned char>(Color.m256d_f64[GreenChannelIndex]);

				constexpr std::size_t RedBufferOffset = 3;
				constexpr std::size_t RedChannelIndex = 3;
				ColorBuffer[Light.LightIndex * Stride + RedBufferOffset] = static_cast<unsigned char>(Color.m256d_f64[RedChannelIndex]);

				constexpr std::size_t BlueBufferOffset = 4;
				constexpr std::size_t BlueChannelIndex = 1;
				ColorBuffer[Light.LightIndex * Stride + BlueBufferOffset] = static_cast<unsigned char>(Color.m256d_f64[BlueChannelIndex]);
			}
		}

		void ClearBuffer()
		{
			// clear the buffer except for the report id and channel info at the front
			memset(ColorBuffer, 0, LightInfo.BufferSize);
			ColorBuffer[0] = LightInfo.ReportId;
			ColorBuffer[1] = LightInfo.Channel;
		}
	};
}
