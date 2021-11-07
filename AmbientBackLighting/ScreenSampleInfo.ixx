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
}
