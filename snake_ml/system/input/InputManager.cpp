// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InputManager.h"

#include "system/drivers/OSDriver.h"
#include "system/drivers/RenderDriver.h"
#include "system/drivers/win/os/helpers/win_utils.h"

namespace snakeml
{

InputManager::InputManager()
{
	IOSDriver::GetInstance()->m_onInputEvent.Subscribe(this, std::bind(&InputManager::ProcessInput, this, std::placeholders::_1));
}

void InputManager::ProcessInput(uint64_t inputKey)
{
	const InputKey key = static_cast<InputKey>(inputKey);
	m_onInputEvent.Dispatch(key);
}

}
