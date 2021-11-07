module;
#include <wchar.h>

export module AmbientBackLighting.Config;
import std.core;

export namespace ABL
{
	enum class LightStripAlignment
	{
		Top = 0,
		Left,
		Right,
		Bottom
	};

	struct LightStripInfo
	{
		unsigned short vendor_id = 0;
		unsigned short product_id = 0;
		const wchar_t* serial;
		std::size_t buffer_size = 0; //TODO: we should be able to derive this...
		//TODO: also figure out how the buffer's report id and channel should be set.

		std::size_t light_count = 0;

		LightStripAlignment alignment = LightStripAlignment::Top;
	};

	struct Config
	{
		//TODO: we have GOT to get rid of this random math in here. it's more or less the size of the buffer?
		LightStripInfo Lights[3] =
		{
			{0x20a0, 0x41e5, L"BS021580-3.1", 2 + 3 * 32, 25, LightStripAlignment::Top},
			{0x20a0, 0x41e5, L"BS021630-3.1", 2 + 3 * 32, 11, LightStripAlignment::Left},
			{0x20a0, 0x41e5, L"BS021581-3.1", 2 + 3 * 32, 11, LightStripAlignment::Right}
		};

		std::size_t SampleThickness = 30; //width in pixels of our rectangular samples

		//old gamma values from main game playthrough: (G: 2.4, R: 1.9, B: 3.4)
		//initial B&W playthrough gamma values: (G: 1.9, R: 1.5, B: 2.7)

		float GammaG = 2.05f;
		float GammaR = 1.8f;
		float GammaB = 2.5f;
	};

}