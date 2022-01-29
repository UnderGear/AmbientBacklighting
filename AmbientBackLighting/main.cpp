#pragma comment(lib, "comctl32.lib")

#include <windows.h>
#include <shellapi.h>

import std.core;
import AmbientBackLighting.BackLighting;
import Profiler;
import "resource.h";

constexpr UINT TrayMessage = WM_APP;
constexpr UINT ExitMessage = TrayMessage + 1;

//Disgusting global state
NOTIFYICONDATA NotifyData = { sizeof(NotifyData) };
auto IsRunning = true;

void InitializeWindow(HINSTANCE hInstance);
void ShowContextMenu(HWND hWnd);
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	InitializeWindow(hInstance);

	auto BackLighting = ABL::AmbientBackLighting{};

	//Profiler::FrameProfiler Profiler;
	while (IsRunning)
	{
		BackLighting.Update();
		//Profiler.Print();
		//Profiler.Reset();

		//TODO: read from config for our refresh rate again.
		// 1 count of a duration that is 1/120 seconds long
		constexpr auto SleepDuration = std::chrono::duration<double, std::ratio<1, 120>> { 1 };
		std::this_thread::sleep_for(SleepDuration);

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

	return 0;
}

void InitializeWindow(HINSTANCE hInstance)
{
	HWND Dialog = CreateDialog(hInstance,
		MAKEINTRESOURCE(IDD_DIALOG1),
		NULL,
		static_cast<DLGPROC>(DlgProc));

	NotifyData.hWnd = Dialog;
	NotifyData.uID = 0;
	NotifyData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	LoadString(hInstance, IDS_APP_TITLE, NotifyData.szTip, ARRAYSIZE(NotifyData.szTip));
	//TODO: icon.
	NotifyData.uCallbackMessage = TrayMessage;

	Shell_NotifyIcon(NIM_ADD, &NotifyData);
}

void ShowContextMenu(HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, -1, MF_BYPOSITION, ExitMessage, L"E&xit");

		// set window to the foreground or the menu won't disappear when it should
		SetForegroundWindow(hWnd);

		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}
}

// Message handler for the app
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
	}
	return 0;
}
