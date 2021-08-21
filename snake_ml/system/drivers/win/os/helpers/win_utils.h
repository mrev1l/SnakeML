// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "lib_includes/win_includes.h"

#include <string>

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{
class WinUtils
{
public:
	static void ThrowIfFailed(HRESULT hr);
	static void RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName, const WNDPROC& winProc);
	static HWND CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle, uint32_t width, uint32_t height);
	static HANDLE CreateEventHandle();

	/*
	* This function allocates dest array
	*/
	static void StringToWstring(const char* source, wchar_t*& dest);
	static void StringToWstring(const char* source, std::wstring& dest);
	static void LoadFileIntoBuffer(const char* filename, std::string& buffer);

private:
	~WinUtils() = delete;
};

}
#endif
}
