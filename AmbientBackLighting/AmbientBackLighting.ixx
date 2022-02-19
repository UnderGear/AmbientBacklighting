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

		ABL::Config AppConfig;
		std::vector<ABL::AmbientLightStripSegment> LightStripSegments;
		bool AreLightsEnabled = true;

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

// 			std::for_each(std::execution::par_unseq, LightStripSegments.begin(), LightStripSegments.end(), [&](auto& LightSegment)
// 			{
// 				LightSegment.Update(AppConfig);
// 			});
			for (auto& LightSegment : LightStripSegments)
			{
				LightSegment.Update(AppConfig);
			}

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
			for (uint32 i = 0; i < ChannelCount; ++i)
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

			// Create light strip segments with screen sampling info and spans into our buffer for their light samples
			auto Index = StartFrameByteCount;
			LightStripSegments.reserve(AppConfig.LightSegments.size());
			for (auto& LightSegmentInfo : AppConfig.LightSegments)
			{
				auto SegmentStartIndex = Index;
				auto SegmentByteCount = LightSegmentInfo.LightCount * Config::BytesPerLight;
				Index = SegmentStartIndex + SegmentByteCount;

				// Create a span into the buffer for this light segment
				std::span SegmentSpan{ BufferSpan.subspan(SegmentStartIndex, Index - SegmentStartIndex) };

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

			for (auto& LightSegment : LightStripSegments)
			{
				LightSegment.ClearBuffer();
			}

			FlushBuffer();
		}

		void SetBrightness(std::uint8_t NewBrightness)
		{
			for (auto& LightSegment : LightStripSegments)
			{
				LightSegment.SetBrightness(NewBrightness);
			}
		}
	};
}
