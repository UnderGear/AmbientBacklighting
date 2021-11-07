module;
#include <windows.h>
#include "hidapi.h"

export module AmbientBackLighting.BackLighting;
import AmbientBackLighting.Config;
import AmbientBackLighting.ImageSummarizer;
import AmbientBackLighting.AmbientLightStrip;
import std.core;

export namespace ABL
{
	class AmbientBackLighting
	{
	public:
		void Update()
		{
			auto Window = GetDesktopWindow();

			for (auto& Light : LightStrips)
			{
				Light->Update(Window, *ImageSummarizer, AppConfig);
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

			auto nScreenWidth = static_cast<std::size_t>(GetSystemMetrics(SM_CXSCREEN));
			auto nScreenHeight = static_cast<std::size_t>(GetSystemMetrics(SM_CYSCREEN));

			//TODO: monitor devices, update these as necessary
			for (auto& LightInfo : AppConfig.Lights)
			{
				if (auto* Device = hid_open(LightInfo.VendorId, LightInfo.ProductId, LightInfo.Serial))
				{
					LightStrips.push_back(std::make_unique<ABL::AmbientLightStrip>(Device, LightInfo, nScreenWidth, nScreenHeight, AppConfig.SampleThickness));
				}
			}
		};

	protected:

		ABL::Config AppConfig;
		std::unique_ptr<ABL::IImageSummarizer> ImageSummarizer;
		std::vector<std::unique_ptr<ABL::AmbientLightStrip>> LightStrips;
	};
}
