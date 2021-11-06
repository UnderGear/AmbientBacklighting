module;

#include <windows.h>
#include "hidapi.h"
import AmbientBackLighting.Config;
import AmbientBackLighting.WindowData;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.AmbientLightStrip;
import std.core;

export module AmbientBackLighting.BackLighting;

export namespace ABL
{
	class AmbientBackLighting
	{
	public:
		void Update()
		{
			auto Window = GetDesktopWindow();

			//WindowSelector.Update();

			//if (WindowSelector.HasValidSelection())
				//Window = WindowSelector.GetSelectedWindow();

			//TODO: remove this after testing.
			//Window = GetForegroundWindow();


			for (auto& Light : LightStrips)
			{
				Light->Update(Window, *ImageSummarizer, AppConfig);
				//don't trust the light to clear the samples, I guess.
				ImageSummarizer->ClearSamples();
			}

			//TODO: do we want to do some HID monitoring in here? if we find one we're looking for, create it?
			//likewise, destroy removed devices
			/*
			// Enumerate and print the HID devices on the system
			struct hid_device_info *devs, *cur_dev;

			devs = hid_enumerate(0x0, 0x0);
			cur_dev = devs;
			while (cur_dev) {
				printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
				cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
				printf("\n");
				printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
				printf("  Product:      %ls\n", cur_dev->product_string);
				printf("\n");
				cur_dev = cur_dev->next;
			}
			hid_free_enumeration(devs);

			int res;
		#define MAX_STR 255
			wchar_t wstr[MAX_STR];

			// Read the Manufacturer String
			res = hid_get_manufacturer_string(DeviceHandle, wstr, MAX_STR);
			printf("Manufacturer String: %ls\n", wstr);

			// Read the Product String
			res = hid_get_product_string(DeviceHandle, wstr, MAX_STR);
			printf("Product String: %ls\n", wstr);

			// Read the Serial Number String
			res = hid_get_serial_number_string(DeviceHandle, wstr, MAX_STR);
			printf("Serial Number String: %ls", wstr);
			printf("\n");

			unsigned char CurrentProMode[2] = { 0x4, 0 };
			auto GetModeResult = hid_get_feature_report(DeviceHandle, CurrentProMode, 2);

			// Set # of LEDs
			unsigned char LEDCountBuf[2] = { 0x81, 32 };// config.HorizontalLightCount }; //TODO: get rid of narrowing warning
			auto SetLEDCountResult = hid_send_feature_report(DeviceHandle, LEDCountBuf, 2);

			// Get # of LEDs
			unsigned char CurrentLEDCount[2] = { 0x81, 0 };
			auto LEDCountResult = hid_get_feature_report(DeviceHandle, CurrentLEDCount, 2);*/
		}

		AmbientBackLighting()
		{
			AppConfig = Config{};
			//TODO: let's pass in CL args or read from a config file and populate our config struct that way.;

			/*std::vector<Color> Seeds
			{
				{0, 255, 255}, //aqua
				{255, 0, 0}, //red
				{0, 255, 0}, //green
				{0, 0, 255}, //blue
				{255, 255, 255}, //white
				{255, 0, 255}, //purple
				{255, 255, 0}, //yellow
				{0, 0, 0}, //black
			};
			ImageSummarizer = new VoronoiImageSummarizer{ Seeds };*/

			//TODO: it'd be good to have an enum in the config that tells us which implementation to use.
			constexpr auto UseRootMeanSquare = true;
			ImageSummarizer = std::make_unique<ABL::AverageImageSummarizer>(UseRootMeanSquare);

			//ImageSummarizer = new AverageLUVImageSummarizer();


			auto nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
			auto nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

			//TODO: report ID should probably come from the library, based on the # of lights in config

			//TODO: monitor devices, update these as necessary

			for (auto& LightInfo : AppConfig.Lights)
			{
				if (auto* Device = hid_open(LightInfo.vendor_id, LightInfo.product_id, LightInfo.serial))
				{
					auto IsVertical = LightInfo.alignment == ABL::LightStripAlignment::Left || LightInfo.alignment == ABL::LightStripAlignment::Right;
					auto SampleInfo = ABL::ScreenSampleInfo
					{
						IsVertical,
						(int)(IsVertical ? AppConfig.SampleThickness : nScreenWidth),
						(int)(IsVertical ? nScreenHeight : AppConfig.SampleThickness),
						(int)(LightInfo.alignment == ABL::LightStripAlignment::Right ? nScreenWidth - AppConfig.SampleThickness : 0),
						(int)(LightInfo.alignment == ABL::LightStripAlignment::Bottom ? nScreenHeight - AppConfig.SampleThickness : 0)
					};
					LightStrips.push_back(std::make_unique<AmbientLightStrip>(Device, LightInfo.light_count, LightInfo.buffer_size, SampleInfo));
				}
			}
		};

		unsigned long GetRefreshMilliseconds() const { return 1000 / AppConfig.SamplesPerSecond; }

	protected:

		ABL::Config AppConfig;
		ABL::WindowSelector WindowSelector;
		std::unique_ptr<ABL::IImageSummarizer> ImageSummarizer;
		std::vector<std::unique_ptr<ABL::AmbientLightStrip>> LightStrips;
	};
}