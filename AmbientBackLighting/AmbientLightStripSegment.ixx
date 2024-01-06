module;
#include <windows.h>

export module AmbientBackLighting.AmbientLightStrip;
import AmbientBackLighting.ScreenSampleInfo;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Config;
import AmbientBackLighting.Light;
import md_span;
import Profiler;
import std.core;

export namespace ABL
{
	class AmbientLightStripSegment
	{
		// Sample data for this light strip
		ABL::ScreenSampleInfo SampleInfo;
		// Sample data per light in this strip
		std::vector<ABL::Light> Lights;

		// Screen sample
		std::vector<RGBQUAD> ScreenSample;
		ABL::md_span<RGBQUAD> SampleSpan;
		BITMAPINFO BitmapInfo;

		HWND& Window;
		HDC WindowDC;
		HDC CaptureDC;
		HBITMAP CaptureBitmap;

	public:
		AmbientLightStripSegment(
			HWND& InWindow, const ABL::LightStripInfo& LightInfo, std::span<uint8_t> BufferSpan,
			int ScreenWidth, int ScreenHeight, int SampleThickness)
			: Window{ InWindow }
		{
			const auto IsVertical = LightInfo.Alignment == ABL::LightSampleAlignment::Left || LightInfo.Alignment == ABL::LightSampleAlignment::Right;
			SampleInfo = ABL::ScreenSampleInfo
			{
				// width, height, offset x, offset y, is vertical
				IsVertical ? SampleThickness : ScreenWidth,
				IsVertical ? ScreenHeight : SampleThickness,
				LightInfo.Alignment == ABL::LightSampleAlignment::Right ? ScreenWidth - SampleThickness : 0,
				LightInfo.Alignment == ABL::LightSampleAlignment::Bottom ? ScreenHeight - SampleThickness : 0,
				IsVertical
			};

			ScreenSample.resize(SampleInfo.SampleWidth * SampleInfo.SampleHeight, {});
			SampleSpan = { ScreenSample.data(), SampleInfo.SampleWidth, SampleInfo.SampleHeight, SampleInfo.SampleWidth, SampleInfo.SampleHeight };

			auto Spacing = static_cast<int>(SampleInfo.IsVertical
				? static_cast<float>(SampleInfo.SampleHeight) / static_cast<float>(LightInfo.LightCount)
				: static_cast<float>(SampleInfo.SampleWidth) / static_cast<float>(LightInfo.LightCount));
			auto Padding = (Spacing - SampleThickness) / 2;

			Lights.reserve(LightInfo.LightCount);
			for (auto LightIndex = 0; LightIndex < LightInfo.LightCount; ++LightIndex)
			{
				auto AdjustedLightIndex = LightIndex;
				//TODO: this is a gross hack that needs to go away.
				if (LightInfo.Alignment == ABL::LightSampleAlignment::Right)
				{
					AdjustedLightIndex = LightInfo.LightCount - LightIndex - 1;
				}

				auto BufferStartIndex = AdjustedLightIndex * Config::BytesPerLight;

				auto StartOffset = LightIndex * (SampleThickness + Padding * 2) + Padding;

				auto SampleStartX = SampleInfo.IsVertical ? 0 : StartOffset;
				auto SampleStartY = SampleInfo.IsVertical ? StartOffset : 0;
				Lights.emplace_back(BufferSpan.subspan(BufferStartIndex, Config::BytesPerLight)
					, SampleSpan.subspan(SampleThickness, SampleThickness, SampleStartX, SampleStartY)
					, SampleThickness * SampleThickness);
			}

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

		~AmbientLightStripSegment()
		{
			ReleaseDC(Window, WindowDC);
			DeleteDC(CaptureDC);
			DeleteObject(CaptureBitmap);
		}

		void ClearBuffer()
		{
			for (auto& Light : Lights)
			{
				Light.ClearBuffer();
			}
		}

		void Update(const ABL::Config& Config)
		{
			//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::Update" };

			{
				//take a snip of the screen for this strip
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateScreenSample" };
				UpdateScreenSample();
			}

			//calculate the summarized color for each light
			for (auto& Light : Lights)
			{
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateLight" };
				Light.Update(Config);
			}
		}

		// [0, 31]
		void SetBrightness(std::uint8_t NewBrightness)
		{
			for (auto& Light : Lights)
			{
				Light.SetBrightness(NewBrightness);
			}
		}

	private:

		void UpdateScreenSample()
		{
			SelectObject(CaptureDC, CaptureBitmap);
			BitBlt(CaptureDC, 0, 0, SampleInfo.SampleWidth, SampleInfo.SampleHeight, WindowDC, SampleInfo.SampleOffsetX, SampleInfo.SampleOffsetY, SRCCOPY | CAPTUREBLT);
			GetDIBits(CaptureDC, CaptureBitmap, 0, SampleInfo.SampleHeight, ScreenSample.data(), &BitmapInfo, DIB_RGB_COLORS);
		}
	};
}
