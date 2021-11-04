#pragma once

#ifdef _WINDOWS
#include "lib_includes/win_includes.h"
#include "lib_includes/directX_includes.h"
#endif

namespace snakeml
{

// in the future this enum needs to be organized in a way to compile in cross-platform project (using defines)

// key mappings from windows defines to project defines
// uses ascii alternatives
enum class InputKey : uint64_t
{
#ifdef _WINDOWS
	ESC					= VK_ESCAPE,
	F11					= VK_F11,
	Num1				= VK_NUMPAD1,
	Num2				= VK_NUMPAD2,

	W					= 0x57,
	S					= 0x53,
	D					= 0x44,
	A					= 0x41,

	GamepadDpadLeft		= XINPUT_GAMEPAD_DPAD_LEFT,
	GamepadDpadRight	= XINPUT_GAMEPAD_DPAD_RIGHT,
	GamepadDpadUp		= XINPUT_GAMEPAD_DPAD_UP,
	GamepadDpadDown		= XINPUT_GAMEPAD_DPAD_DOWN,
#endif
};

}