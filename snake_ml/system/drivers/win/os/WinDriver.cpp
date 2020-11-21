// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "WinDriver.h"

#include "system/drivers/win/dx/DX12Driver.h"

#include <Shlwapi.h>

namespace snakeml
{
namespace system
{
namespace win
{

WinDriver::WinDriver(const wchar_t* windowClassName, const wchar_t* windowTitle, math::vec2<uint32_t> windowSz)
	: IOSDriver()
	, m_windowSz(windowSz)
{
	const HINSTANCE hInstance = GetHInstance();
	WinUtils::RegisterWindowClass(hInstance, windowClassName, &WndProc);
	m_windowHandle = WinUtils::CreateWindow(windowClassName, hInstance, windowTitle, windowSz.m_x, windowSz.m_y);
}

WinDriver::~WinDriver()
{
	delete system::IRenderDriver::GetInstance();
}

void WinDriver::OnInitialize()
{
	// Set the working directory to the path of the executable.
	WCHAR path[MAX_PATH];
	HMODULE hModule = GetModuleHandleW(NULL);
	if (GetModuleFileNameW(hModule, path, MAX_PATH) > 0)
	{
		PathRemoveFileSpecW(path);
		SetCurrentDirectoryW(path);
	}

	new system::win::DX12Driver(m_windowHandle, m_windowSz);
	system::IRenderDriver::GetInstance()->Initialize();
}

void WinDriver::OnRunOSMainLoop()
{
	::ShowWindow(m_windowHandle, SW_SHOW);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	::PostQuitMessage(0);
}

HINSTANCE WinDriver::GetHInstance()
{
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;
	HMODULE hm = 0;
	::GetModuleHandleEx(flags, reinterpret_cast<LPCTSTR>(WinDriver::GetHInstance), &hm);
	return hm;
}

LRESULT WinDriver::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	IOSDriver* osDriver = IOSDriver::GetInstance();
	if (system::IRenderDriver::GetInstance() && system::IRenderDriver::GetInstance()->IsInitialized())
	{
		switch (message)
		{
		case WM_PAINT:
			osDriver->m_onUpdateEvent.Dispatch();
			break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			osDriver->m_onInputEvent.Dispatch(wParam);
			break;
			// The default window procedure will play a system notification sound 
			// when pressing the Alt+Enter keyboard combination if this message is 
			// not handled.
		case WM_SYSCHAR:
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);;
			break;
		default:
			return ::DefWindowProcW(hwnd, message, wParam, lParam);
		}
	}
	else
	{
		return ::DefWindowProcW(hwnd, message, wParam, lParam);
	}

	return 0;
}

void WinDriver::OnQuit()
{
	::PostQuitMessage(0);
}

}
}
}
