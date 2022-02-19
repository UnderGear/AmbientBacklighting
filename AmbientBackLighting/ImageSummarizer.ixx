module;
#include "immintrin.h"
#include "ammintrin.h"

export module AmbientBackLighting.ImageSummarizer;
import std.core;

export namespace ABL
{
	class RGBSampler
	{
		__m256d Value{ 0 };
		uint64_t SampleCount = 0;

		//TODO: I hate this bool. do we really need to have this anyway? can you really tell the difference?
		bool UseRootMeanSquare = true;

	public:

		void ClearSamples()
		{
			Value = _mm256_set1_pd(0.0);
			SampleCount = 0;
		}

		void SetSampleCount(uint64_t NewSampleCount) { SampleCount = NewSampleCount; }
		uint64_t GetSampleCount() const { return SampleCount; }

		void AddSample(double R, double G, double B)
		{
			__m256d SourceColor = _mm256_set_pd(R, G, B, 0.0);
			if (UseRootMeanSquare)
			{
				Value = _mm256_macc_pd(SourceColor, SourceColor, Value);
			}
			else
			{
				Value = _mm256_add_pd(SourceColor, Value);
			}
		}
	
		__m256d GetColor(__m256d Gammas) const
		{
			auto Result = _mm256_div_pd(Value, _mm256_set1_pd(static_cast<double>(SampleCount)));

			if (UseRootMeanSquare)
			{
				Result = _mm256_sqrt_pd(Result);
			}

			return GammaCorrect(Result, Gammas);
		}

		static constexpr double MinColorValue = 0.0;
		static constexpr double MaxColorValue = 255.0;
		static constexpr double Half = 0.5;
		inline static const __m256d Maxes = _mm256_set1_pd(MaxColorValue);
		inline static const __m256d Halves = _mm256_set1_pd(Half);

		__m256d GammaCorrect(__m256d RawValues, __m256d Gammas) const
		{
			//Profiler::StackFrameProfile StackFrame = { "AmbientLightStrip::GammaCorrect", 5 };

			//corrected value is (Index/255)^Gamma * Max + 0.5
			auto CorrectedValues = _mm256_pow_pd(_mm256_div_pd(RawValues, Maxes), Gammas);
			CorrectedValues = _mm256_macc_pd(CorrectedValues, Maxes, Halves);

			return CorrectedValues;
		}
	};
}
