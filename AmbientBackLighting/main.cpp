#pragma comment(lib, "comctl32.lib")

#include <windows.h>
#include <shellapi.h>

import std;
import AmbientBackLighting.BackLighting;
import Profiler;
import "resource.h";

constexpr UINT TrayMessage = WM_APP;
constexpr UINT ToggleMessage = TrayMessage + 1;
constexpr UINT ExitMessage = TrayMessage + 2;

//Disgusting global state so the winproc can actually work
//TODO: I'm considering pulling the windows crap into its own class that holds a unique ptr to a backlighting
// and the windows instance can be a gross raw pointer we new and delete in here.
NOTIFYICONDATA NotifyData = { sizeof(NotifyData) };
auto IsScreenOn = true;
auto IsRunning = true;
auto IsActive = true;
ABL::AmbientBackLighting* BackLighting = new ABL::AmbientBackLighting;

void InitializeWindow(HINSTANCE hInstance);
void ShowContextMenu(HWND hWnd);
INT_PTR CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	InitializeWindow(hInstance);

	//Profiler::FrameProfiler Profiler;
	while (BackLighting != nullptr && IsRunning)
	{
		if (IsScreenOn && IsActive)
		{
			BackLighting->Update();
			//Profiler.Print();
			//Profiler.Reset();
		}
		else
		{
			BackLighting->DisableLights();
		}

		//TODO: read from config for our refresh rate again.
		// 1 count of a duration that is 1/120 seconds long
		//constexpr auto SleepDuration = std::chrono::duration<double, std::ratio<1, 120>>{ 1 };
		//std::this_thread::sleep_for(SleepDuration);

		MSG Message;
		while (PeekMessage(&Message, nullptr, 0, 0, PM_NOREMOVE) > 0)
		{
			if (GetMessage(&Message, NULL, 0, 0))
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
	}

	// because of the WM_ENDSESSION case in WinProc, we had to make BackLighting a pointer
	// and not even a unique_ptr.
	// so now we have to manually delete it here if we're going through the normal flow.
	if (BackLighting != nullptr)
	{
		delete BackLighting;
	}

	return 0;
}

void InitializeWindow(HINSTANCE hInstance)
{
	HWND Dialog = CreateDialog(hInstance,
		MAKEINTRESOURCE(IDD_DIALOG1),
		NULL,
		static_cast<DLGPROC>(WinProc));

	RegisterPowerSettingNotification(Dialog, &GUID_CONSOLE_DISPLAY_STATE, DEVICE_NOTIFY_WINDOW_HANDLE);

	NotifyData.hWnd = Dialog;
	NotifyData.uID = 0;
	NotifyData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	LoadString(hInstance, IDS_APP_TITLE, NotifyData.szTip, ARRAYSIZE(NotifyData.szTip));
	NotifyData.hIcon = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR));
	NotifyData.uCallbackMessage = TrayMessage;

	Shell_NotifyIcon(NIM_ADD, &NotifyData);

	// manually clean up the icon behind us
	if (NotifyData.hIcon && DestroyIcon(NotifyData.hIcon))
		NotifyData.hIcon = NULL;
}

void ShowContextMenu(HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, -1, MF_BYPOSITION, ToggleMessage, L"&Toggle");
		InsertMenu(hMenu, -1, MF_BYPOSITION, ExitMessage, L"E&xit");

		// set window to the foreground or the menu won't disappear when it should
		SetForegroundWindow(hWnd);

		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}
}

// Message handler for the app
INT_PTR CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case TrayMessage:
	{
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
			ShowContextMenu(hWnd);
		}
		break;
	}
	case WM_COMMAND:
	{
		auto wmId = LOWORD(wParam);
		switch (wmId)
		{
		case ExitMessage:
			DestroyWindow(hWnd);
			break;
		case ToggleMessage:
			IsActive = !IsActive;
			break;
		}
		return 1;
	}
	case WM_DESTROY:
	{
		NotifyData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &NotifyData);
		PostQuitMessage(0);
		IsRunning = false;
		break;
	}
	case WM_QUERYENDSESSION:
	{
		return 1;
	}
	case WM_ENDSESSION:
	{
		// immediately delete the backlighting to call its dtor.
		// we don't get any other messages after this; windows kills the process.
		NotifyData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &NotifyData);
		delete BackLighting;
		return 1;
	}
	case WM_POWERBROADCAST:
	{
		// toggle IsScreenOn depending on display state changes
		auto Event = LOWORD(wParam);
		switch (Event)
		{
		case PBT_POWERSETTINGCHANGE:
		{
			//TODO: support more power settings?
			//TODO: wake up doesn't seem to be working correctly
			auto* PowerSetting = reinterpret_cast<POWERBROADCAST_SETTING*>(lParam);
			if (PowerSetting->PowerSetting == GUID_CONSOLE_DISPLAY_STATE)
			{
				IsScreenOn = PowerSetting->Data[0] != 0;
				return 1;
			}
		}
		}		
		
		break;
	}
	case WM_DISPLAYCHANGE:
		auto Horizontal = LOWORD(lParam);
		auto Vertical = HIWORD(lParam);
		IsScreenOn = Horizontal > 0 && Vertical > 0;
		break;
	}
	return 0;
}
