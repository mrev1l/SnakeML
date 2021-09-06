// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InputHandlingSystem.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/components/DebugRenderComponent.h"
#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/InputDataComponent.h"

namespace snakeml
{

InputHandlingSystem::InputHandlingSystem() : ISystemCastableImpl<InputHandlingSystem>()
{
	InputManager::GetInstance()->m_onActionReleased.Subscribe(this, std::bind(&InputHandlingSystem::OnInput, this, std::placeholders::_1));
	InputManager::GetInstance()->m_onInputAxisEvent.Subscribe(this, std::bind(&InputHandlingSystem::OnAxis, this, std::placeholders::_1));
}

void InputHandlingSystem::Update(float dt)
{
	InputDataComponentIterator& it = *ECSManager::GetInstance()->GetComponents<InputDataComponentIterator>();
	ASSERT(it.Size() == 1u, "[InputHandlingSystem::PopulateInputVector] : Multiple entities storing input.");
	InputDataComponent& inputData = it.At(0);
	
	for (auto it = inputData.m_axesInput.rbegin(); it != inputData.m_axesInput.rend(); ++it)
	{
		InputManager::InputAxisData inputAxis = *it;
		if (!IsNearlyZero(inputAxis.value))
		{
			EntityControllerComponentIterator& it = *ECSManager::GetInstance()->GetComponents<EntityControllerComponentIterator>();
			ASSERT(it.Size() == 1u, "[InputHandlingSystem::Update] : Multiple entities receiving input.");

			const vector inputDirection = inputAxis.axis == InputAxis::MoveForward ? vector::up : vector::right;
			it.At(0).m_inputVector += inputDirection * inputAxis.value;
		}
	}
	inputData.m_axesInput.clear();
}

void InputHandlingSystem::OnInput(InputAction inputEvent) const
{
	switch (inputEvent)
	{
	case InputAction::DebugRendering: ToggleDebugRendering(); break;
	default:
		break;
	}
}

void InputHandlingSystem::OnAxis(InputManager::InputAxisData inputAxis) const
{
	switch (inputAxis.axis)
	{
	case InputAxis::MoveForward:
	case InputAxis::MoveRight:		PopulateInputVector(inputAxis); break;
	}
}

void InputHandlingSystem::ToggleDebugRendering() const
{
	DebugRenderComponentIterator* it = ECSManager::GetInstance()->GetComponents<DebugRenderComponentIterator>();
	for (DebugRenderComponent& debugRenderComp : *it)
	{
		debugRenderComp.m_isEnabled = !debugRenderComp.m_isEnabled;
	}
}

void InputHandlingSystem::PopulateInputVector(InputManager::InputAxisData inputAxis) const
{
	InputDataComponentIterator& it = *ECSManager::GetInstance()->GetComponents<InputDataComponentIterator>();
	ASSERT(it.Size() == 1u, "[InputHandlingSystem::PopulateInputVector] : Multiple entities storing input.");

	it.At(0).m_axesInput.push_back(inputAxis);
}

}