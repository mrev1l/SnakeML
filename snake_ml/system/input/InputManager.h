// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "InputKeys.h"

#include "utils/patterns/event.h"
#include "utils/patterns/singleton.h"

namespace snakeml
{

enum class InputEvent
{
	Pressed,
	Released,
};

enum class InputAction
{
	Exit,
	DebugRendering,

	Invalid
};

enum class InputAxis
{
	MoveForward,
	MoveRight,

	Invalid
};

class InputManager : public Singleton<InputManager>
{
public:
	struct InputAxisData
	{
		InputAxis	axis;
		float		value;
	};

	InputManager();
	~InputManager() = default;

	void Update(float dt);

	Event<InputManager, InputAction>	m_onActionPressed;
	Event<InputManager, InputAction>	m_onActionReleased;
	Event<InputManager, InputAxisData>	m_onInputAxisEvent;

private:
	void ProcessKeyDown(uint64_t inputKey);
	void ProcessKeyUp(uint64_t inputKey);

	void ProcessInput_Action(InputAction, InputEvent);
	void ProcessInput_Axis(InputAxisData);

	std::unordered_map<InputAxis, InputAxisData>	m_activeAxes;
	std::vector<InputAction>						m_pressedActions;
	std::vector<InputAction>						m_releasedActions;

	static InputAction GetInputAction(InputKey);
	static InputAxisData GetInputAxis(InputKey);

	static std::unordered_map<InputKey, InputAction>	s_actionsMappings;
	static std::unordered_map<InputKey, InputAxisData>	s_axesMappings;
};

}
