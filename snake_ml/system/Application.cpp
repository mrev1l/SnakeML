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
#include "ecs/systems/ChildControllerSystem.h"
#include "ecs/systems/ConsumablesSystem.h"
#include "ecs/systems/InitializeDebugRenderComponentsSystem.h"
#include "ecs/systems/EntityControllerSystem.h"
#include "ecs/systems/InitializePhysicsComponentsSystem.h"
#include "ecs/systems/InputHandlingSystem.h"
#include "ecs/systems/LevelLoadingSystem.h"
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
	constexpr uint32_t2 windowSz = { 640, 640 };

#ifdef _WINDOWS
	new win::WinDriver(windowClassName.data(), windowTitle.data(), windowSz);
#endif
	IOSDriver::GetInstance()->Initialize();
	IOSDriver::GetInstance()->m_onUpdateEvent.Subscribe(this, std::bind(&Application::Update, this, std::placeholders::_1));

	new InputManager();
	InputManager::GetInstance()->m_onActionReleased.Subscribe(this, std::bind(&Application::OnInput, this, std::placeholders::_1));

	new ECSManager();

	std::unique_ptr<LevelLoadingSystem> levelLoadingSystem = std::make_unique<LevelLoadingSystem>();
	levelLoadingSystem->m_onLoadingComplete.Subscribe(this, []() -> void
		{
#ifdef _WINDOWS
			ECSManager::GetInstance()->ExecuteSystem(std::make_unique<win::InitializeRenderComponentsSystem>());
#endif
			ECSManager::GetInstance()->ExecuteSystem(std::make_unique<InitializePhysicsComponentsSystem>());
			ECSManager::GetInstance()->ExecuteSystem(std::make_unique<InitializeDebugRenderComponentsSystem>());
		});
	ECSManager::GetInstance()->ScheduleSystem(std::move(levelLoadingSystem));


	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<wip::WIP_System>());
	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<InputHandlingSystem>());
	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<EntityControllerSystem>());
	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<PhysicsSystem>());
	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<ChildControllerSystem>());
	ECSManager::GetInstance()->ScheduleSystem(std::make_unique<ConsumablesSystem>()); // TODO : ordering!
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

void Application::OnInput(InputAction inputEvent)
{
	if (inputEvent == InputAction::Exit)
	{
		IOSDriver::GetInstance()->Quit();
	}
}

void Application::Update(float dt)
{
	static uint64_t frameCounter = 0;
	static float elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;
	auto t1 = clock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;
	elapsedSeconds += deltaTime.count() * 1e-9f;
	if (elapsedSeconds > 1.0)
	{
		wchar_t buffer[500];
		auto fps = frameCounter / elapsedSeconds;
		swprintf_s(buffer, 500, L"FPS: %f\n", fps);
		IOSDriver::GetInstance()->LogMessage(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}
	InputManager::GetInstance()->Update(dt);
	ECSManager::GetInstance()->Update(dt);
	Render();
}

void Application::Render()
{
	IRenderDriver::GetInstance()->Render();
}

}
