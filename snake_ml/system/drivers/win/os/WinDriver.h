// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "system/drivers/OSDriver.h"
#include "system/drivers/win/os/helpers/win_utils.h"

#include "utils/types/HighResolutionClock.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

using WindowHandle = HWND;

class WinDriver
	: public IOSDriver
{
public:
	WinDriver(const wchar_t* windowClassName, const wchar_t* windowTitle, uint32_t2 windowSz);
	~WinDriver();

	void GetAppDimensions(uint32_t& _outWidth, uint32_t& _outHeight) const override;

	void LogMessage(std::wstring message) const override;

private:
	static HINSTANCE GetHInstance();
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void OnInitialize() override;
	void OnRunOSMainLoop() override;
	void OnQuit() override;

	void OnUpdate() override;

	HWND m_windowHandle;
	uint32_t2 m_windowSz;
	HighResolutionClock m_updateClock;
};

}
#endif
}
