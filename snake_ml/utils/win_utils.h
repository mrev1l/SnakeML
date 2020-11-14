// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "lib_includes/win_includes.h"

namespace snakeml
{

class WinUtils
{
public:
	static void RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName, const WNDPROC& winProc);
	static HWND CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle, uint32_t width, uint32_t height);
	static HANDLE CreateEventHandle();
private:
	~WinUtils() = delete;
};

// key mappings from windows defines to project defines
enum class SupportedKey : uint64_t
{
	ESC = VK_ESCAPE,
	F11 = VK_F11,
};

}