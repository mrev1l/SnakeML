#pragma once

namespace snakeml
{
namespace types
{

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