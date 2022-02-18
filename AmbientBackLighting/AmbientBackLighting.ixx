module;
#include <windows.h>
#include "ftd2xx.h"
#include "libMPSSE_spi.h"

export module AmbientBackLighting.BackLighting;
import AmbientBackLighting.Config;
import AmbientBackLighting.AmbientLightStrip;
import std.core;

export namespace ABL
{
	class AmbientBackLighting
	{
		//TODO: we've got this in a couple of places. just read it from config?
		static constexpr std::uint8_t MaxBrightness = 0b00011111; // 31

		ABL::Config AppConfig;
		std::vector<ABL::AmbientLightStripSegment> LightStripSegments;
		std::uint8_t Brightness = MaxBrightness;
		bool AreLightsEnabled = true;
		FT_HANDLE ftHandle;

		// The data to send to the LED strip
		// Contains a Start Frame section, a Color section, and an End Frame section
		// Start Frame
		//  - 4 bytes of 0x0
		// Color section
		//  - n Colors, where n is the number of LEDs we're controlling
		//    - Color are composed of a Header byte followed by Blue, Red, and Green channel bytes
		//      - Header Byte
		//        - 0b11100000 required section
		//        - [0, 31] global brightness setting for the LED (note that 31 = 0b00011111)
		//        - These two sections are ORed together to produce the header byte
		//      - Blue channel [0, 255]
		//      - Red channel [0, 255]
		//      - Green channel [0, 255]
		// End Frame
		//  - at least n/2 bits of 0x0s (or 0x1s, just don't mix and match)
		std::vector<std::uint8_t> Buffer;

		// Send our Buffer's current value over the wire to the LED strip
		void FlushBuffer()
		{
			uint32 SentCount = 0;
			auto Status = SPI_Write(ftHandle, Buffer.data(), static_cast<uint32>(Buffer.size()), &SentCount, SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
		}

	public:
		void Update()
		{
			AreLightsEnabled = true;

			std::for_each(std::execution::par_unseq, LightStripSegments.begin(), LightStripSegments.end(), [&](auto& LightStripSegment)
			{
				LightStripSegment.Update(AppConfig);
			});

			FlushBuffer();
		}

		AmbientBackLighting()
		{
			AppConfig = Config{};
			//TODO: let's pass in CL args or read from a config file and populate our config struct that way.

			auto ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
			auto ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

			auto Window = GetDesktopWindow();

			constexpr uint8 latency = 1;
			ChannelConfig channelConf = { 0 };
			channelConf.ClockRate = FT_BAUD_921600;
			channelConf.LatencyTimer = latency;
			channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3;
			channelConf.Pin = 0;

			Init_libMPSSE();

			//TODO: pull this out to its own method
			uint32 ChannelIndex = 0;
			FT_DEVICE_LIST_INFO_NODE DeviceInfo{ 0 };
			uint32 ChannelCount = 0;
			auto Status = SPI_GetNumChannels(&ChannelCount);
			for (uint32 i = 0; i < ChannelCount; ++i) //TODO: can we move this to some sort of algo, maybe backed by ranges?
			{
				Status = SPI_GetChannelInfo(i, &DeviceInfo);

				if (DeviceInfo.ID == AppConfig.DeviceId)
				{
					ChannelIndex = i;
					break;
				}
			}

			Status = SPI_OpenChannel(ChannelIndex, &ftHandle);
			Status = SPI_InitChannel(ftHandle, &channelConf);

			auto TotalLightCount = AppConfig.GetTotalLightCount();

			constexpr std::size_t StartFrameByteCount = 4;
			std::size_t EndFrameByteCount = (TotalLightCount + 15Ui64) / 16Ui64;

			auto BufferSize = StartFrameByteCount + TotalLightCount * Config::BytesPerLight + EndFrameByteCount;
			Buffer.resize(BufferSize, 0);
			std::span BufferSpan{ Buffer }; //TODO: is there a cleaner way of initializing the other spans than calling subspan on this?

			// Fill our buffer.
			// Skip bytes for the start frame. We've zero-filled that in our resize.
			// Similarly, our end frame is zero-filled as well.
			std::size_t Index = StartFrameByteCount;
			LightStripSegments.reserve(AppConfig.LightSegments.size());
			for (auto& LightSegmentInfo : AppConfig.LightSegments)
			{
				std::size_t StartIndex = Index;
				std::size_t EndIndex = StartIndex + LightSegmentInfo.LightCount * Config::BytesPerLight;
				for (; Index < EndIndex; Index += Config::BytesPerLight)
				{
					Buffer[Index] = 255;
				}

				// Create a span into the buffer for this light segment
				std::span SegmentSpan{ BufferSpan.subspan(StartIndex, EndIndex - StartIndex) };
				LightStripSegments.emplace_back(Window, LightSegmentInfo, SegmentSpan, ScreenWidth, ScreenHeight, AppConfig.SampleThickness);
			}

			SetBrightness(AppConfig.Brightness);
		};

		~AmbientBackLighting()
		{
			DisableLights();

			auto Status = SPI_CloseChannel(ftHandle);

			Cleanup_libMPSSE();
		}

		void DisableLights()
		{
			if (AreLightsEnabled == false)
				return;

			AreLightsEnabled = false;

			std::for_each(std::execution::par_unseq, LightStripSegments.begin(), LightStripSegments.end(), [&](auto& LightStripSegment)
			{
				LightStripSegment.ClearBuffer();
			});

			FlushBuffer();
		}

		void SetBrightness(std::uint8_t NewBrightness)
		{
			auto ClampedNewBrightness = std::clamp(NewBrightness, 0Ui8, MaxBrightness);
			if (ClampedNewBrightness == Brightness)
				return;

			Brightness = ClampedNewBrightness;

			std::for_each(std::execution::par_unseq, LightStripSegments.begin(), LightStripSegments.end(), [&](auto& LightStripSegment)
			{
				LightStripSegment.SetBrightness(Brightness);
			});
		}
	};
}
