module;
#include <windows.h>

export module AmbientBackLighting.AmbientLightStrip;
import AmbientBackLighting.ScreenSampleInfo;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Config;
import AmbientBackLighting.Light;
import Profiler;
import std.core;

export namespace ABL
{
	class AmbientLightStripSegment
	{
		// Span into our Buffer for writing to the device
		// The span only exposes the subset of the buffer's bytes this segment should actually be able to access
		std::span<uint8_t> BufferSpan;

		static constexpr std::uint8_t HeaderTopBits = 0b11100000;

		// Light config data
		const ABL::LightStripInfo& LightInfo;
		// Sample data for this light strip
		ABL::ScreenSampleInfo SampleInfo;
		ABL::RGBSampler Sampler = {};
		// Sample data per light in this strip
		std::vector<ABL::LightSampleInfo> Lights;

		// Screen sample
		std::vector<RGBQUAD> ScreenSample;
		BITMAPINFO BitmapInfo;

		HWND& Window;
		HDC WindowDC;
		HDC CaptureDC;
		HBITMAP CaptureBitmap;

	public:
		AmbientLightStripSegment(
			HWND& InWindow, const ABL::LightStripInfo& InLightInfo, std::span<uint8_t> InBufferSpan,
			int ScreenWidth, int ScreenHeight, int SampleThickness)
			: BufferSpan{ InBufferSpan }, LightInfo{ InLightInfo }, Window{ InWindow }
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

			const auto Spacing = static_cast<std::size_t>(SampleInfo.IsVertical
				? static_cast<float>(SampleInfo.SampleHeight) / static_cast<float>(LightInfo.LightCount)
				: static_cast<float>(SampleInfo.SampleWidth) / static_cast<float>(LightInfo.LightCount));
			const auto Padding = (Spacing - SampleThickness) / 2;

			Lights.reserve(LightInfo.LightCount);
			for (std::size_t LightIndex = 0; LightIndex < LightInfo.LightCount; ++LightIndex)
			{
				auto BufferStartIndex = LightIndex * Config::BytesPerLight;

				//TODO: I really hate this solution. it should depend on the winding at least
				//TODO: if we're on the right side, we'll need to inverse our light index vs sample or at least know we need to walk it backwards
				auto AdjustedLightIndex = LightIndex;
				if (LightInfo.Alignment == ABL::LightSampleAlignment::Right)
				{
					AdjustedLightIndex = LightInfo.LightCount - LightIndex - 1;
				}

				Lights.emplace_back(AdjustedLightIndex, BufferSpan.subspan(BufferStartIndex, Config::BytesPerLight), SampleInfo, SampleThickness, Padding);
			}

			ScreenSample.resize(SampleInfo.SampleWidth* SampleInfo.SampleHeight, {});
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
			ClearBuffer();

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
			//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::Update", 0 };

			{
				//take a snip of the screen for this strip
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateScreenSample", 1 };
				UpdateScreenSample();
			}

			//calculate the summarized color for each light
			for (auto& Light : Lights)
			{
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateLight", 2 };
				UpdateLight(Config, Light);
			}
		}

		// [0, 31]
		void SetBrightness(std::uint8_t NewBrightness)
		{
			// Color headers are composed of a required 3 1 bits followed by 5 brightness bits
			auto Header = HeaderTopBits | NewBrightness;
			for (auto& Light : Lights)
			{
				Light.SetHeader(Header);
			}
		}

	private:

		void UpdateScreenSample()
		{
			SelectObject(CaptureDC, CaptureBitmap);
			BitBlt(CaptureDC, 0, 0, SampleInfo.SampleWidth, SampleInfo.SampleHeight, WindowDC, SampleInfo.SampleOffsetX, SampleInfo.SampleOffsetY, SRCCOPY | CAPTUREBLT);
			GetDIBits(CaptureDC, CaptureBitmap, 0, SampleInfo.SampleHeight, ScreenSample.data(), &BitmapInfo, DIB_RGB_COLORS);
		}

		void UpdateLight(const ABL::Config& Config, ABL::LightSampleInfo& Light)
		{
			// TODO: this is a great use case of an mdspan.
			// add the color of every pixel in our screen sample associated with this light to our image summarizer
			for (auto x = Light.SampleStartX; x < Light.SampleEndX; ++x)
			{
				for (auto y = Light.SampleStartY; y < Light.SampleEndY; ++y)
				{
					const auto SampleIndex = x + y * SampleInfo.SampleWidth - 1; // 2D -> 1D array index conversion
					const auto& SamplePixel = ScreenSample[SampleIndex];
					//Profiler::StackFrameProfile StackFrame = { "ImageSummarizer::AddSample", 4 };
					Sampler.AddSample(static_cast<double>(SamplePixel.rgbRed), static_cast<double>(SamplePixel.rgbGreen), static_cast<double>(SamplePixel.rgbBlue));
				}
			}

			auto Color = Sampler.GetColor(Config.Gammas);
			Sampler.ClearSamples();

			{
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateLight - Update Buffer", 6 };
				Light.SetColor(Color);
			}
		}
	};
}
