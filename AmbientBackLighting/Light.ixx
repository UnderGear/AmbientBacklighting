module;
#include "immintrin.h"
#include "ammintrin.h"
#include <windows.h>

export module AmbientBackLighting.Light;
import AmbientBackLighting.ScreenSampleInfo;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.Config;
import md_span;
import std.core;

export namespace ABL
{
	extern constexpr std::size_t HeaderChannelIndex = 0Ui64;
	extern constexpr std::size_t BlueChannelIndex = 1Ui64;
	extern constexpr std::size_t RedChannelIndex = 2Ui64;
	extern constexpr std::size_t GreenChannelIndex = 3Ui64;

	extern constexpr std::uint8_t HeaderTopBits = 0b11100000Ui8;
	extern constexpr std::uint8_t MaxBrightness = 0b00011111Ui8; // 31

	class Light
	{
		std::span<uint8_t> BufferSpan;
		ABL::md_span<RGBQUAD> SampleSpan;

		ABL::RGBSampler Sampler;

		std::uint8_t Brightness = MaxBrightness;

	public:
		constexpr Light(std::span<uint8_t> InBufferSpan, ABL::md_span<RGBQUAD> InSampleSpan, int SampleCount)
			: BufferSpan{ InBufferSpan }, SampleSpan{ InSampleSpan }, Sampler{ SampleCount }
		{ }

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
			auto ClampedNewBrightness = std::clamp(NewBrightness, 0Ui8, MaxBrightness);
			if (ClampedNewBrightness == Brightness)
				return;

			Brightness = ClampedNewBrightness;

			// Color headers are composed of a required 3 1 bits followed by 5 brightness bits
			auto Header = HeaderTopBits | Brightness;
			BufferSpan[HeaderChannelIndex] = Header;
		}

		void Update(const ABL::Config& Config)
		{
			// TODO: can this use an accumulate instead of this for loop? kill off the sampler entirely?
			// add the color of every pixel in our screen sample associated with this light to our image summarizer
			for (auto& SamplePixel : SampleSpan)
			{
				Sampler.AddSample(static_cast<double>(SamplePixel.rgbRed), static_cast<double>(SamplePixel.rgbGreen), static_cast<double>(SamplePixel.rgbBlue));
			}

			auto Color = Sampler.GetColor(Config.Gammas);
			Sampler.ClearSamples();

			{
				//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::UpdateLight - Update Buffer", 6 };
				SetColor(Color);
			}
		}

	};
}
