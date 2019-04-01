#include "stdafx.h"
#include "WindowData.h"
#include <algorithm>

static BOOL CALLBACK CheckWindow(HWND hwnd, LPARAM lParam)
{
	auto* WindowCollection = reinterpret_cast<WindowSelector*>(lParam);
	if (WindowCollection == nullptr)
		return false;

	return WindowCollection->CheckWindowMatch(hwnd);
}

void WindowSelector::EnumerateWindows()
{
	Windows.clear();

	EnumWindows(CheckWindow, reinterpret_cast<LPARAM>(this));
}

BOOL WindowSelector::CheckWindowMatch(HWND hwnd)
{
	if (IsWindowVisible(hwnd) == false)
		return true;

	auto ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	auto ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	auto WindowWidth = windowRect.right - windowRect.left;
	auto WindowHeight = windowRect.bottom - windowRect.top;

	auto bMatch = WindowWidth == ScreenWidth && WindowHeight == ScreenHeight;

	if (bMatch)
	{
		WCHAR title[1024];
		GetWindowTextW(hwnd, title, 1024);
		std::wstring titleString{ title };
		std::string trimmedTitle{ titleString.begin(), titleString.end() };
		std::replace(trimmedTitle.begin(), trimmedTitle.end(), '\'', '\0');

		auto Data = WindowData{};

		GetWindowThreadProcessId(hwnd, &Data.pId);

		Data.Handle = hwnd;
		Data.Width = ScreenWidth;
		Data.Height = ScreenHeight;
		Data.Title = trimmedTitle;
		Windows.push_back(Data);

		//std::cout << "Title: " << trimmedTitle << std::endl;
		//std::wcout << L"    Left: " << windowRect.left << L", Right: " << windowRect.right << L", Top: " << windowRect.top << L", Bottom: " << windowRect.bottom << std::endl;

	}

	return true;
}

void WindowSelector::UpdateSelection()
{
	for (unsigned int i = 0; i < 10; ++i)
	{
		auto Character = '0' + i;
		if (GetKeyState(Character) & 0x8000)
		{
			Index = i;
		}
	}
}

void WindowSelector::Update()
{
	EnumerateWindows();
	UpdateSelection();
}
