// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InputManager.h"
#include "system/drivers/OSDriver.h"
#include "system/drivers/RenderDriver.h"
#include "utils/win_utils.h"

namespace snakeml
{
namespace system
{

InputManager::InputManager()
{
	IOSDriver::GetInstance()->m_onInputEvent.Subscribe(this, std::bind(&InputManager::ProcessInput, this, std::placeholders::_1));
}

void InputManager::ProcessInput(uint64_t inputKey)
{
	switch (static_cast<winutils::SupportedKey>(inputKey))
	{
	case winutils::SupportedKey::ESC: IOSDriver::GetInstance()->Quit(); break;
	//case SupportedKey::F11: system::RenderDriver::GetInstance()->SetFullscreenMode(!system::RenderDriver::GetInstance()->IsInFullscreen(), system::OSDriver::GetInstance()->GetWindowHandle()); break;
	default:
		break;
	}
}

}
}
