#pragma once
#include "lib_includes/win_includes.h"

namespace snakeml
{

// in the future this enum needs to be organized in a way to compile in cross-platform project (using defines)

// key mappings from windows defines to project defines
// uses ascii alternatives
enum class InputKey : uint64_t
{
#ifdef _WINDOWS
	ESC		= VK_ESCAPE,
	F11		= VK_F11,
	W		= 0x57,
	S		= 0x53,
	D		= 0x44,
	A		= 0x41,
	Num1	= VK_NUMPAD1,
#endif
};

}