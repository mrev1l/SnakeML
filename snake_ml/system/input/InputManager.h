// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "InputKeys.h"

#include "utils/patterns/event.h"
#include "utils/patterns/singleton.h"

namespace snakeml
{

class InputManager
	: public Singleton<InputManager>
{
public:
	InputManager();
	~InputManager() = default;

	Event<InputManager, InputKey> m_onInputEvent;

private:
	void ProcessInput(uint64_t inputKey);
};

}
