export module AmbientBackLighting.ScreenSampleInfo;
import std.core;

export namespace ABL
{
	struct ScreenSampleInfo
	{
		bool IsVertical = false;
		std::size_t SampleWidth = 0;
		std::size_t SampleHeight = 0;
		std::size_t SampleOffsetX = 0;
		std::size_t SampleOffsetY = 0;
	};

	struct LightSampleInfo
	{
		std::size_t LightIndex = 0;
		std::size_t SampleStartX = 0;
		std::size_t SampleEndX = 0;
		std::size_t SampleStartY = 0;
		std::size_t SampleEndY = 0;

		constexpr LightSampleInfo(std::size_t InLightIndex, const ABL::ScreenSampleInfo& SampleInfo, float Spacing)
			: LightIndex(InLightIndex)
		{
			SampleStartX = SampleInfo.IsVertical ? 0 : static_cast<std::size_t>(std::round(Spacing * LightIndex));
			SampleEndX = SampleInfo.IsVertical ? SampleInfo.SampleWidth : static_cast<std::size_t>(std::round(SampleStartX + Spacing));
			SampleStartY = SampleInfo.IsVertical ? static_cast<std::size_t>(std::round(Spacing * LightIndex)) : 0;
			SampleEndY = SampleInfo.IsVertical ? static_cast<std::size_t>(std::round(SampleStartY + Spacing)) : SampleInfo.SampleHeight;
		}
	};
}
