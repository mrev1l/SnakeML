// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "Application.h"

#ifdef _WINDOWS
#include "drivers/win/dx/DX12Driver.h"
#include "drivers/win/os/WinDriver.h"
#include "ecs/systems/platform_specific/win/InitializeRenderComponentsSystem.h"
#endif

#include "ecs/ECSManager.h"
#include "ecs/systems/InitializeCameraSystem.h"
#include "ecs/systems/InitializePhysicsComponentsSystem.h"
#include "ecs/systems/LoadMaterialsSystem.h"
#include "ecs/systems/PhysicsSystem.h"
#include "ecs/systems/Render2DSystem.h"
#include "ecs/systems/WIP_System.h"

#include "input/InputManager.h"

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
	constexpr uint32_t2 windowSz = { 720, 720 };

#ifdef _WINDOWS
	new win::WinDriver(windowClassName.data(), windowTitle.data(), windowSz);
#endif
	new InputManager();

	IOSDriver::GetInstance()->Initialize();

	IOSDriver::GetInstance()->m_onUpdateEvent.Subscribe(this, std::bind(&Application::Update, this, std::placeholders::_1));

	new ECSManager();

	ECSManager::GetInstance()->ExecuteSystem(std::make_unique<LoadMaterialsSystem>());
	ECSManager::GetInstance()->ExecuteSystem(std::make_unique<win::InitializeRenderComponentsSystem>());
	ECSManager::GetInstance()->ExecuteSystem(std::make_unique<InitializeCameraSystem>());
	ECSManager::GetInstance()->ExecuteSystem(std::make_unique<InitializePhysicsComponentsSystem>());

	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<wip::WIP_System>());
	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<PhysicsSystem>());
	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<Render2DSystem>());
}

void Application::Run()
{
	IOSDriver::GetInstance()->RunOSMainLoop();
}

void Application::Shutdown()
{
	ECSManager* ecsManager = ECSManager::GetInstance();
	IOSDriver* osDriver = IOSDriver::GetInstance();
	InputManager* inputMgr = InputManager::GetInstance();

	delete ecsManager;
	delete osDriver;
	delete inputMgr;
}

void Application::Update(double dt)
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
		IOSDriver::GetInstance()->LogMessage(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}

	ECSManager::GetInstance()->Update(dt);
	Render();
}

void Application::Render()
{
	IRenderDriver::GetInstance()->Render();
}

}
