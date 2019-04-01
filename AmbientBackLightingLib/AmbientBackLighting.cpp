#include "stdafx.h"
#include "AmbientBackLighting.h"
#include "AmbientLightStrip.h"
#include "ScreenSampleInfo.h"

AmbientBackLighting::AmbientBackLighting(Config InConfig)
{
	AppConfig = InConfig;
	//TODO: let's pass in CL args or read from a config file and populate our config struct that way.;

	//TODO: listen for screen size changes and recalculate stuff as necessary
	nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	verticalSpacing = nScreenHeight / AppConfig.VerticalLightCount;
	horizontalSpacing = nScreenWidth / AppConfig.HorizontalLightCount;
	
	//TODO: read from config? maybe calculate based on config and looking up in the library?
	BufferSize = 2 + 3 * 32;

	//TODO: report ID should probably come from the library, based on the # of lights in config

	//TODO: monitor devices, update these as necessary
	//TODO: move the serial numbers into config

	if (auto* TopDevice = hid_open(0x20a0, 0x41e5, TEXT("BS021580-3.1")))
	{
		auto SampleInfo = ScreenSampleInfo{};
		SampleInfo.SampleWidth = nScreenWidth;
		SampleInfo.SampleHeight = AppConfig.SampleThickness;
		SampleInfo.SampleOffsetX = 0;
		SampleInfo.SampleOffsetY = 0;
		LightStripTop = new AmbientLightStrip(TopDevice, AppConfig.HorizontalLightCount, BufferSize, SampleInfo);
	}

	if (auto* LeftDevice = hid_open(0x20a0, 0x41e5, TEXT("BS021630-3.1")))
	{
		auto SampleInfo = ScreenSampleInfo{};
		SampleInfo.SampleWidth = AppConfig.SampleThickness;
		SampleInfo.SampleHeight = nScreenHeight;
		SampleInfo.SampleOffsetX = 0;
		SampleInfo.SampleOffsetY = 0;
		SampleInfo.IsVertical = true;
		LightStripLeft = new AmbientLightStrip(LeftDevice, AppConfig.VerticalLightCount, BufferSize, SampleInfo);
	}

	if (auto* RightDevice = hid_open(0x20a0, 0x41e5, TEXT("BS021581-3.1")))
	{
		auto SampleInfo = ScreenSampleInfo{};
		SampleInfo.SampleWidth = AppConfig.SampleThickness;
		SampleInfo.SampleHeight = nScreenHeight;
		SampleInfo.SampleOffsetX = nScreenWidth - AppConfig.SampleThickness;
		SampleInfo.SampleOffsetY = 0;
		SampleInfo.IsVertical = true;
		LightStripRight = new AmbientLightStrip(RightDevice, AppConfig.VerticalLightCount, BufferSize, SampleInfo);
	}
};

AmbientBackLighting::~AmbientBackLighting()
{
	delete LightStripTop;
	delete LightStripLeft;
	delete LightStripRight;
};

void AmbientBackLighting::Update(float DeltaTime)
{
	auto Window = GetDesktopWindow();

	WindowSelector.Update();

	if (WindowSelector.HasValidSelection())
		Window = WindowSelector.GetSelectedWindow();

	if (LightStripTop)
		LightStripTop->Update(DeltaTime, Window);

	if (LightStripLeft)
		LightStripLeft->Update(DeltaTime, Window);

	if (LightStripRight)
		LightStripRight->Update(DeltaTime, Window);

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
