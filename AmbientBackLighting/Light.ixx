module;
#include "immintrin.h"
#include "ammintrin.h"
#include <windows.h>

export module AmbientBackLighting.Light;
import AmbientBackLighting.ScreenSampleInfo;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Config;
import std.core;

export namespace ABL
{
	static constexpr std::size_t HeaderChannelIndex = 0;
	static constexpr std::size_t BlueChannelIndex = 1;
	static constexpr std::size_t RedChannelIndex = 2;
	static constexpr std::size_t GreenChannelIndex = 3;

	static constexpr std::uint8_t HeaderTopBits = 0b11100000;

	struct LightSampleInfo
	{
		std::span<uint8_t> BufferSpan;

		std::size_t SampleStartX = 0;
		std::size_t SampleEndX = 0;
		std::size_t SampleStartY = 0;
		std::size_t SampleEndY = 0;

		ABL::RGBSampler Sampler = {};

		//TODO: this would be a great use case for a multi dimensional span.
		constexpr LightSampleInfo(std::size_t LightIndex, std::span<uint8_t> InBufferSpan, const ABL::ScreenSampleInfo& SampleInfo, std::size_t SampleThickness, std::size_t Padding)
			: BufferSpan{ InBufferSpan }
		{
			// the full spacing taken up per light is: pad, sample, pad. we want N spacings plus another padding to get to our sample at this index
			auto StartOffset = LightIndex * (SampleThickness + Padding * 2) + Padding;
			auto EndOffset = StartOffset + SampleThickness;

			SampleStartX = SampleInfo.IsVertical ? 0 : StartOffset;
			SampleEndX = SampleInfo.IsVertical ? SampleInfo.SampleWidth : EndOffset;
			SampleStartY = SampleInfo.IsVertical ? StartOffset : 0;
			SampleEndY = SampleInfo.IsVertical ? EndOffset : SampleInfo.SampleHeight;
		}

		void ClearBuffer()
		{
			BufferSpan[BlueChannelIndex] = 0;
			BufferSpan[RedChannelIndex] = 0;
			BufferSpan[GreenChannelIndex] = 0;
		}

		void Update(const ABL::Config& Config, const ABL::ScreenSampleInfo& SampleInfo, const std::vector<RGBQUAD>& ScreenSample)
		{
			// TODO: this is a great use case of an mdspan.
			// add the color of every pixel in our screen sample associated with this light to our image summarizer
			for (auto x = SampleStartX; x < SampleEndX; ++x)
			{
				for (auto y = SampleStartY; y < SampleEndY; ++y)
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
				BufferSpan[BlueChannelIndex] = static_cast<std::uint8_t>(Color.m256d_f64[BlueChannelIndex]);
				BufferSpan[RedChannelIndex] = static_cast<std::uint8_t>(Color.m256d_f64[RedChannelIndex]);
				BufferSpan[GreenChannelIndex] = static_cast<std::uint8_t>(Color.m256d_f64[GreenChannelIndex]);
			}
		}

		void SetBrightness(uint8_t NewBrightness)
		{
			// Color headers are composed of a required 3 1 bits followed by 5 brightness bits
			auto Header = HeaderTopBits | NewBrightness;
			BufferSpan[HeaderChannelIndex] = Header;
		}

	};
}
