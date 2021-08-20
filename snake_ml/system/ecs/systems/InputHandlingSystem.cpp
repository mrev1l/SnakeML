// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InputHandlingSystem.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/components/DebugRenderComponent.h"

#include "system/input/InputManager.h"

namespace snakeml
{

InputHandlingSystem::InputHandlingSystem()
{
	InputManager::GetInstance()->m_onInputEvent.Subscribe(this, std::bind(&InputHandlingSystem::OnInput, this, std::placeholders::_1));
}

void InputHandlingSystem::OnInput(InputKey inputKey)
{
	switch (inputKey)
	{
	case snakeml::InputKey::D: ToggleDebugRendering(); break;
	default:
		break;
	}
}

void InputHandlingSystem::ToggleDebugRendering()
{
	DebugRenderComponentIterator* it = ECSManager::GetInstance()->GetComponents<DebugRenderComponentIterator>();
	for (DebugRenderComponent& debugRenderComp : *it)
	{
		debugRenderComp.m_isEnabled = !debugRenderComp.m_isEnabled;
	}
}

}