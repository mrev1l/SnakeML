// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "Application.h"

#ifdef _WINDOWS
#include "drivers/win/dx/DX12Driver.h"
#include "drivers/win/os/WinDriver.h"
#include "ecs/systems/platform_specific/InitializeRenderComponentsSystem.h"
#include "ecs/systems/platform_specific/LoadMaterialsSystem.h"
#endif

#include "ecs/ECSManager.h"
#include "ecs/systems/InitializeCameraSystem.h"
#include "ecs/systems/InitializeEntitiesSystem.h"
#include "ecs/systems/InitializeTransformComponentsSystem.h"
#include "ecs/systems/RotateCubeSystem.h"
#include "ecs/systems/Render2DSystem.h"
#include "ecs/systems/TestMoveSnakeHeadSystem.h"
#include "input/InputManager.h"

#include <functional>
#include <string_view>

namespace snakeml
{
uint64_t Application::s_frameCounter = 0;

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
	constexpr math::vec2<uint32_t> windowSz = {720, 720};

#ifdef _WINDOWS
	new system::win::WinDriver(windowClassName.data(), windowTitle.data(), windowSz);
#endif
	new InputManager();

	system::IOSDriver::GetInstance()->Initialize();

	system::IOSDriver::GetInstance()->m_onUpdateEvent.Subscribe(this, std::bind(&Application::Update, this));

	new system::ECSManager();
	// TODO : Clean up
	//system::InitializeCubeMaterialSystem* sys1 = new system::InitializeCubeMaterialSystem();
	system::InitializeRenderComponentsSystem* sys2 = new system::InitializeRenderComponentsSystem();
	system::InitializeTransformComponentsSystem* sys3 = new system::InitializeTransformComponentsSystem();
	system::InitializeCameraSystem initCameraSystem;
	system::InitializeEntitiesSystem* sys4 = new system::InitializeEntitiesSystem();
	system::LoadMaterialsSystem* sys5 = new system::LoadMaterialsSystem();
	//system::ECSManager::GetInstance()->ExecuteSystem(sys1);
	system::ECSManager::GetInstance()->ExecuteSystem(sys5);
	system::ECSManager::GetInstance()->ExecuteSystem(sys2);
	system::ECSManager::GetInstance()->ExecuteSystem(sys3);
	system::ECSManager::GetInstance()->ExecuteSystem(&initCameraSystem);
	system::ECSManager::GetInstance()->ExecuteSystem(sys4);
	//system::ECSManager::GetInstance()->ScheduleSystem(new system::RotateCubeSystem());
	system::ECSManager::GetInstance()->ScheduleSystem(new system::TestMoveSnakeHeadSystem());
	system::ECSManager::GetInstance()->ScheduleSystem(new system::Render2DSystem());
	//delete sys1;
	delete sys2;
	delete sys3;
	delete sys4;
	delete sys5;
}

void Application::Run()
{
	system::IOSDriver::GetInstance()->RunOSMainLoop();
}

void Application::Shutdown()
{
	system::ECSManager* ecsManager = system::ECSManager::GetInstance();
	system::IOSDriver* osDriver = system::IOSDriver::GetInstance();
	InputManager* inputMgr = InputManager::GetInstance();

	delete ecsManager;
	delete osDriver;
	delete inputMgr;
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

	system::ECSManager::GetInstance()->Update();
	Render();
	++s_frameCounter;
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
