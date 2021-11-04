// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InputManager.h"

#include "system/drivers/OSDriver.h"

namespace snakeml
{

std::unordered_map<InputKey, InputAction> InputManager::s_actionsMappings =
{
	{ InputKey::ESC,	InputAction::Exit },
	{ InputKey::Num1,	InputAction::DebugRendering },
	{ InputKey::Num2,	InputAction::DebugSpawnSnakeChild }
};

std::unordered_map<InputKey, InputManager::InputAxisData> InputManager::s_axesMappings =
{
	{ InputKey::W,					{ InputAxis::MoveForward,	+1.f } },
	{ InputKey::GamepadDpadUp,		{ InputAxis::MoveForward,	+1.f } },

	{ InputKey::S,					{ InputAxis::MoveForward,	-1.f } },
	{ InputKey::GamepadDpadDown,	{ InputAxis::MoveForward,	-1.f } },

	{ InputKey::D,					{ InputAxis::MoveRight,		+1.f } },
	{ InputKey::GamepadDpadRight,	{ InputAxis::MoveRight,		+1.f } },

	{ InputKey::A,					{ InputAxis::MoveRight,		-1.f } },
	{ InputKey::GamepadDpadLeft,	{ InputAxis::MoveRight,		-1.f } }
};

InputManager::InputManager()
{
	IOSDriver::GetInstance()->m_onKeyDownEvent.Subscribe(this, std::bind(&InputManager::ProcessKeyDown, this, std::placeholders::_1));
	IOSDriver::GetInstance()->m_onKeyUpEvent.Subscribe(this, std::bind(&InputManager::ProcessKeyUp, this, std::placeholders::_1));
}

void InputManager::Update(float dt)
{
	for (std::pair<const InputAxis, InputAxisData>& activeAxis : m_activeAxes)
	{
		m_onInputAxisEvent.Dispatch(activeAxis.second);
	}

	for (const InputAction action : m_pressedActions)
	{
		m_onActionPressed.Dispatch(action);
	}
	m_pressedActions.clear();

	for (const InputAction action : m_releasedActions)
	{
		m_onActionReleased.Dispatch(action);
	}
	m_releasedActions.clear();
}

void InputManager::ProcessKeyDown(uint64_t inputKey)
{
	const InputKey key = static_cast<InputKey>(inputKey);
	const InputAction inputAction = GetInputAction(key);
	const InputAxisData inputAxisData = GetInputAxis(key);

	ASSERT(inputAction == InputAction::Invalid || inputAxisData.axis == InputAxis::Invalid, "[InputManager::ProcessKeyDown] : Ambiguous input.");
	
	if (inputAction != InputAction::Invalid)
	{
		ProcessInput_Action(inputAction, InputEvent::Pressed);
	}
	if (inputAxisData.axis != InputAxis::Invalid)
	{
		ProcessInput_Axis(inputAxisData);
	}
}

void InputManager::ProcessKeyUp(uint64_t inputKey)
{
	const InputKey key = static_cast<InputKey>(inputKey);
	const InputAction inputAction = GetInputAction(key);
	InputAxisData inputAxisData = GetInputAxis(key);
	inputAxisData.value = 0.f;

	ASSERT(inputAction == InputAction::Invalid || inputAxisData.axis == InputAxis::Invalid, "[InputManager::ProcessKeyDown] : Ambiguous input.");

	if (inputAction != InputAction::Invalid)
	{
		ProcessInput_Action(inputAction, InputEvent::Released);
	}
	if (inputAxisData.axis != InputAxis::Invalid)
	{
		ProcessInput_Axis(inputAxisData);
	}
}

void InputManager::ProcessInput_Action(InputAction action, InputEvent actionEvent)
{
	const auto ProcessAction = [] (std::vector<InputAction>& actions, const InputAction action) -> void
	{
		const bool isActionPresent = std::find(actions.begin(), actions.end(), action) != actions.end();
		ASSERT(!isActionPresent, "[InputManager::ProcessInput_Action] : Obtained multiple instance of the same input action in a single frame.");

		actions.push_back(action);
	};

	if (actionEvent == InputEvent::Pressed)
	{
		ProcessAction(m_pressedActions, action);
	}
	if (actionEvent == InputEvent::Released)
	{
		ProcessAction(m_releasedActions, action);
	}
}

void InputManager::ProcessInput_Axis(InputAxisData axisData)
{
	m_activeAxes[axisData.axis] = axisData;
}

InputAction InputManager::GetInputAction(InputKey key)
{
	if (s_actionsMappings.contains(key))
	{
		return s_actionsMappings[key];
	}
	return InputAction::Invalid;
}

InputManager::InputAxisData InputManager::GetInputAxis(InputKey key)
{
	if (s_axesMappings.contains(key))
	{
		return s_axesMappings[key];
	}
	return InputAxisData{ InputAxis::Invalid, 0.f };
}

}
