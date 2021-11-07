module;
#include <windows.h>

export module AmbientBackLighting.WindowData;
import std.core;

export namespace ABL
{
	struct WindowData
	{
		HWND Handle;
		int Width = 0;
		int Height = 0;
		std::wstring Title;
		unsigned long pId = 0;
	};

	class WindowSelector
	{
	public:

		void Update()
		{
			EnumerateWindows();
			UpdateSelection();
		}

		bool HasValidSelection() const { return Windows.size() > Index; }
		HWND GetSelectedWindow() { return Windows[Index].Handle; }
		static BOOL CALLBACK CheckWindow(HWND hwnd, LPARAM lParam)
		{
			auto* WindowCollection = reinterpret_cast<WindowSelector*>(lParam);
			if (WindowCollection == nullptr)
				return false;

			return WindowCollection->CheckWindowMatch(hwnd);
		}

		BOOL CheckWindowMatch(HWND hwnd)
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
				wchar_t title[1024];
				GetWindowTextW(hwnd, title, 1024);
				std::wstring titleString{ title };
				// 		std::string trimmedTitle = std::string{ titleString.begin(), titleString.end() };
				// 		std::replace(trimmedTitle.begin(), trimmedTitle.end(), '\'', '\0');

				auto Data = WindowData{};

				GetWindowThreadProcessId(hwnd, &Data.pId);

				Data.Handle = hwnd;
				Data.Width = ScreenWidth;
				Data.Height = ScreenHeight;
				Data.Title = titleString;
				Windows.push_back(Data);

				//std::cout << "Title: " << trimmedTitle << std::endl;
				//std::wcout << L"    Left: " << windowRect.left << L", Right: " << windowRect.right << L", Top: " << windowRect.top << L", Bottom: " << windowRect.bottom << std::endl;

			}

			return true;
		}

	protected:
		std::vector<WindowData> Windows;

		void EnumerateWindows()
		{
			Windows.clear(); //TODO: maybe we don't really need to clear this every time. only add uniques to the list and remove if no longer found?

			auto Data = WindowData{};
			Data.Handle = GetDesktopWindow();
			Windows.push_back(Data);

			EnumWindows(CheckWindow, reinterpret_cast<LPARAM>(this));
		}

		unsigned int Index = 0;

		//TODO: do this on key down callbacks or something instead.
		void UpdateSelection()
		{
			//if (GetKeyState((VK_MENU) & 0x8000) == false)
				//return;

			/*for (unsigned int i = 0; i < 10; ++i)
			{
				auto Character = '0' + i; //kind of a hack, but it'll get the int I need
				if (GetKeyState(Character) & 0x8000)
				{
					Index = i;
				}
			}*/
		}
	};
}
