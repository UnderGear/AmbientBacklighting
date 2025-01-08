export module AmbientBackLighting.ScreenSampleInfo;

export namespace ABL
{
	struct ScreenSampleInfo
	{
		int SampleWidth = 0;
		int SampleHeight = 0;
		int SampleOffsetX = 0;
		int SampleOffsetY = 0;
		bool IsVertical = false;
	};
}
