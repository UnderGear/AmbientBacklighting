#pragma once
#include <vector>
#include <string>

struct EXPORT WindowData
{
	HWND Handle;
	int Width;
	int Height;
	std::string Title;
	unsigned long pId;
};

class EXPORT WindowSelector
{
public:
	void Update();
	bool HasValidSelection() const { return Windows.size() > Index; }
	HWND GetSelectedWindow() { return Windows[Index].Handle; }
	BOOL CheckWindowMatch(HWND hwnd);

protected:
	std::vector<WindowData> Windows;

	void EnumerateWindows();

	unsigned int Index = 0;

	void UpdateSelection();
};
