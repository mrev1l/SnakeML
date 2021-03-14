// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "utils/snake_math.h"
#include "utils/types/arithmetic.h"

#include <cassert>
#include <stdint.h>
#include <memory>

#ifdef NDEBUG
	#define ASSERT(Condition, Message) (Condition);
#else
	#define ASSERT(Condition, Message) if(!(Condition)) { _wassert(_CRT_WIDE(#Message), _CRT_WIDE(__FILE__), __LINE__); }
#endif

#pragma warning(disable : 4834)