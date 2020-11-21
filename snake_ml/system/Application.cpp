// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "Application.h"

#ifdef _WINDOWS
#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/os/WinDriver.h"
#endif

#include "system/input/InputManager.h"

#include <functional>
#include <string_view>

namespace snakeml
{
Application::Application() : Singleton()
{
}

Application::~Application()
{
}

void Application::Initialize()
{
	constexpr std::wstring_view windowClassName(L"SnakeML_WindowClass");
	constexpr std::wstring_view windowTitle(L"snake_ml");
	constexpr math::vec2<uint32_t> windowSz = {1280, 720};

#ifdef _WINDOWS
	new system::win::WinDriver(windowClassName.data(), windowTitle.data(), windowSz);
#endif
	new InputManager();

	system::IOSDriver::GetInstance()->Initialize();

	system::IOSDriver::GetInstance()->m_onUpdateEvent.Subscribe(this, std::bind(&Application::Update, this));
}

void Application::Run()
{
	system::IOSDriver::GetInstance()->RunOSMainLoop();
}

void Application::Shutdown()
{
	system::IRenderDriver::GetInstance()->Shutdown();

	delete system::IOSDriver::GetInstance();

	delete InputManager::GetInstance(); //-V586
}

void Application::Update()
{
	static uint64_t frameCounter = 0;
	static double elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;
	auto t1 = clock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;
	elapsedSeconds += deltaTime.count() * 1e-9;
	if (elapsedSeconds > 1.0)
	{
		wchar_t buffer[500];
		auto fps = frameCounter / elapsedSeconds;
		swprintf_s(buffer, 500, L"FPS: %f\n", fps);
		OutputDebugString(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}

	Render();
}

void Application::Render()
{
	system::IRenderDriver::GetInstance()->Render();
}

void Application::Resize(uint32_t width, uint32_t height)
{
	//system::IRenderDriver::GetInstance()->Resize(width, height);
}

}
