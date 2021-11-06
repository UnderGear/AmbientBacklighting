module;

export module AmbientBackLighting.ScreenSampleInfo;

export namespace ABL
{
	struct ScreenSampleInfo
	{
		bool IsVertical = false;
		int SampleWidth = 0;
		int SampleHeight = 0;
		int SampleOffsetX = 0;
		int SampleOffsetY = 0;
	};
}
