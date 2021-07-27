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
};

inline bool TestIntersection_AABB_AABB(const AABB& a, const AABB& b)
{
	const bool isABehindB = a.min.x >= b.max.x;
	const bool isABeforeB = a.max.x <= b.min.x;
	const bool isAAboveA = a.min.y >= b.max.y;
	const bool isABelowB = a.max.y <= b.min.y;

	return !(isABehindB || isABeforeB || isAAboveA || isABelowB);
}

}
}