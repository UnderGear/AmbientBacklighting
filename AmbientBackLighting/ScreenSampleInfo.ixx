export module AmbientBackLighting.ScreenSampleInfo;
import std.core;

export namespace ABL
{
	struct ScreenSampleInfo
	{
		std::size_t SampleWidth = 0;
		std::size_t SampleHeight = 0;
		std::size_t SampleOffsetX = 0;
		std::size_t SampleOffsetY = 0;
		bool IsVertical = false;
	};

	struct LightSampleInfo
	{
		std::size_t LightIndex = 0;
		std::size_t SampleStartX = 0;
		std::size_t SampleEndX = 0;
		std::size_t SampleStartY = 0;
		std::size_t SampleEndY = 0;

		constexpr LightSampleInfo(std::size_t InLightIndex, const ABL::ScreenSampleInfo& SampleInfo, std::size_t SampleThickness, std::size_t Padding)
			: LightIndex(InLightIndex)
		{
			// the full spacing taken up per light is: pad, sample, pad. we want N spacings plus another padding to get to our sample at this index
			auto StartOffset = LightIndex * (SampleThickness + Padding * 2) + Padding;
			auto EndOffset = StartOffset + SampleThickness;

			SampleStartX = SampleInfo.IsVertical ? 0 : StartOffset;
			SampleEndX = SampleInfo.IsVertical ? SampleInfo.SampleWidth : EndOffset;
			SampleStartY = SampleInfo.IsVertical ? StartOffset : 0;
			SampleEndY = SampleInfo.IsVertical ? EndOffset : SampleInfo.SampleHeight;
		}
	};
}
