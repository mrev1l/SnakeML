// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

namespace snakeml
{
namespace math
{

constexpr float default_epsilon = 1e-4f;

template<typename T>
class vec2
{
public:
	T m_x;
	T m_y;
};

template<typename T>
class vec3
{
public:
	T m_x;
	T m_y;
	T m_z;
};

template<typename T>
class vec4
{
public:
	T m_x;
	T m_y;
	T m_z;
	T m_w;
};

}
}