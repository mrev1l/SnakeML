// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once


#include <algorithm>
#include <array>
#include <cassert>
#include <stdint.h>
#include <map>
#include <math.h>
#include <memory>
#include <vector>
#include <unordered_set>

#ifdef NDEBUG
	#define ASSERT(Condition, Message) (Condition);
#else
	#define ASSERT(Condition, Message) if(!(Condition)) { _wassert(_CRT_WIDE(#Message), _CRT_WIDE(__FILE__), __LINE__); }
#endif

#include "utils/snake_math.h"
#include "utils/math/vector.h"
#include "utils/math/matrix.h"
#include "utils/math/math_utils.h"

#include "utils/types/arithmetic.h"

#pragma warning(disable : 4834)