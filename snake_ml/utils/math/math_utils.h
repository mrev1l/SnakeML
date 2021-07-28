#pragma once

namespace snakeml
{
namespace math
{

constexpr float k_PI        = 3.141592654f;
constexpr float k_2PI       = 6.283185307f;
constexpr float k_1DIVPI    = 0.318309886f;
constexpr float k_1DIV2PI   = 0.159154943f;
constexpr float k_PIDIV2    = 1.570796327f;
constexpr float k_PIDIV4    = 0.785398163f;

inline float ConvertToRadians(float deg) { return deg * (k_PI / 180.0f); }
inline float ConvertToDegrees(float rad) { return rad * (180.0f / k_PI); }

inline math::vector triple_product(const math::vector& a, const math::vector& b, const math::vector& c)
{
	// https://en.wikipedia.org/wiki/Triple_product#Vector_triple_product
	return a.cross(b).cross(c);
}


inline math::vector perpendicular2d(const math::vector& a)
{
	return { -a.y, a.x, 0.f };
}

}
}