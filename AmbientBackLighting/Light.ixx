module;
#include "immintrin.h"
#include "ammintrin.h"

export module AmbientBackLighting.Light;
import AmbientBackLighting.ScreenSampleInfo;
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

		void SetColor(__m256d Color)
		{
			BufferSpan[BlueChannelIndex] = static_cast<std::uint8_t>(Color.m256d_f64[BlueChannelIndex]);
			BufferSpan[RedChannelIndex] = static_cast<std::uint8_t>(Color.m256d_f64[RedChannelIndex]);
			BufferSpan[GreenChannelIndex] = static_cast<std::uint8_t>(Color.m256d_f64[GreenChannelIndex]);
		}

		void SetBrightness(uint8_t NewBrightness)
		{
			// Color headers are composed of a required 3 1 bits followed by 5 brightness bits
			auto Header = HeaderTopBits | NewBrightness;
			BufferSpan[HeaderChannelIndex] = Header;
		}

	};
}
