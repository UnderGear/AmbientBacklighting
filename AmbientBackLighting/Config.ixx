module;
#include "immintrin.h"

export module AmbientBackLighting.Config;
import std.core;

export namespace ABL
{
	enum class LightSampleAlignment
	{
		Top = 0,
		Left,
		Right,
		Bottom
	};

	// From the perspective of looking at your screen from the front with the lights mounted on the back
	enum class LightStripWindingDirection
	{
		LeftToRight = 0,
		RightToLeft,
	};

	struct LightStripInfo
	{
		LightSampleAlignment Alignment = LightSampleAlignment::Top;
		int LightCount = 0;
	};

	struct Config
	{
		static constexpr std::size_t BytesPerLight = 4;

		std::size_t DeviceId = 0x4036001;
		LightStripWindingDirection Winding = LightStripWindingDirection::LeftToRight;
		std::vector<LightStripInfo> LightSegments =
		{
			{ LightSampleAlignment::Left, 12 }, //TODO: 48 per side
			{ LightSampleAlignment::Top, 120 },
			{ LightSampleAlignment::Right, 12 } //TODO: sample from top to bottom!
		};

		auto GetTotalLightCount() const
		{
			return std::accumulate(LightSegments.begin(), LightSegments.end(), 0Ui64,
				[](const std::size_t& Count, const LightStripInfo& SegmentInfo)
				{
					return Count + SegmentInfo.LightCount;
				});
		}

		int SampleThickness = 25; //width in pixels of our rectangular samples

		//old gamma values from main game playthrough: (G: 2.4, R: 1.9, B: 3.4)
		//initial B&W playthrough gamma values: (G: 1.9, R: 1.5, B: 2.7)

		//Global brightness to use, will be clamped to the interval [0, 31]
		std::uint8_t Brightness = 0x1;

		double GammaG = 2.05;
		double GammaR = 1.8;
		double GammaB = 2.5;

		__m256d Gammas;

		Config() : Gammas{ _mm256_set_pd(GammaR, GammaG, GammaB, 0.0) } { }
	};

}