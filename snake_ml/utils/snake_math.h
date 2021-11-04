// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#pragma region (copyright)
/*
 *  Copyright(c) 2017 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file Helpers.h
  *  @date August 28, 2017
  *  @author Jeremiah van Oosten
  *
  *  @brief Helper functions.
  */
#pragma endregion

namespace snakeml
{

constexpr float k_default_epsilon = 1e-4f;
constexpr float k_precise_epsilon = 1e-9f;

/***************************************************************************
	* These functions were taken from the MiniEngine.
	* Source code available here:
	* https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
	* Retrieved: January 13, 2016
	**************************************************************************/
template <typename T>
inline T AlignUpWithMask(T value, size_t mask)
{
	return (T)(((size_t)value + mask) & ~mask);
}

template <typename T>
inline T AlignUp(T value, size_t alignment)
{
	return AlignUpWithMask(value, alignment - 1);
}

template <typename T>
inline T DivideByMultiple(T value, size_t alignment)
{
	return (T)((value + alignment - 1) / alignment);
}

inline bool IsNearlyZero(float val, float epsilon = k_default_epsilon)
{
	return abs(val - 0.f) < epsilon;
}

inline bool IsNearlyEqual(float a, float b, float epsilon = k_default_epsilon)
{
	return std::abs(a - b) < k_default_epsilon;
}

}
