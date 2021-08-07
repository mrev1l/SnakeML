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

// TODO : Move to algorithm space
struct AABB
{
	math::vector min, max;

	static bool TestIntersection_AABB_AABB(const AABB& a, const AABB& b);
	static AABB GenerateAABB(const math::vector& dimensions, const math::vector& position, const math::vector& rotation);
};

}
}