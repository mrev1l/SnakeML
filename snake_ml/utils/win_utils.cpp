// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "win_utils.h"

#pragma warning(push)
#pragma warning( disable : 4365)
#include <algorithm>
#pragma warning(pop)
#include <fstream>

namespace snakeml
{
#ifdef _WINDOWS
namespace winutils
{

void WinUtils::RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName, const WNDPROC& winProc)
{
	// Register a window class for creating our render window with.
	WNDCLASSEXW windowClass = {};

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = winProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInst;
	windowClass.hIcon = ::LoadIcon(hInst, NULL);
	windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = ::LoadIcon(hInst, NULL);

	static ATOM atom = ::RegisterClassExW(&windowClass);
	ASSERT(atom > 0, "");
}

HWND WinUtils::CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle, uint32_t width, uint32_t height)
{
	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	// Center the window within the screen. Clamp to 0, 0 for the top-left corner.
	int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
	int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

	HWND hWnd = ::CreateWindowExW(
		NULL,
		windowClassName,
		windowTitle,
		WS_POPUPWINDOW,
		windowX,
		windowY,
		windowWidth,
		windowHeight,
		NULL,
		NULL,
		hInst,
		nullptr
	);

	ASSERT(hWnd, "Failed to create window");

	return hWnd;
}

HANDLE WinUtils::CreateEventHandle()
{
	HANDLE eventHandle;

	eventHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(eventHandle, "Failed to create event.");

	return eventHandle;
}

void WinUtils::StringToWstring(const char* source, wchar_t*& dest)
{
	int destSz = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
	dest = new wchar_t[static_cast<size_t>(destSz)];
	MultiByteToWideChar(CP_UTF8, 0, source, -1, dest, destSz);
}

void WinUtils::StringToWstring(const char* source, std::wstring& dest)
{
	wchar_t* wStr = nullptr;
	WinUtils::StringToWstring(source, wStr);
	dest = std::wstring(wStr); // TODO: we don't actually copy it in ctor do we?
	delete wStr;
}

void WinUtils::LoadFileIntoBuffer(const char* filename, std::string& fileBuffer)
{
	std::ifstream materialJsonFile(filename, std::ios_base::in);
	
	std::string line;
	while (materialJsonFile >> line)
	{
		fileBuffer.append(line);
	}
}

}
#endif
}
