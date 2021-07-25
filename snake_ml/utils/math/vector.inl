#pragma once

inline const snakeml::math::vector snakeml::math::vector::forward	= vector{ +0.f, +0.f, +1.f };
inline const snakeml::math::vector snakeml::math::vector::backward	= vector{ +0.f, +0.f, -1.f };
inline const snakeml::math::vector snakeml::math::vector::up		= vector{ +0.f, +1.f, +0.f };
inline const snakeml::math::vector snakeml::math::vector::down		= vector{ +0.f, -1.f, +0.f };
inline const snakeml::math::vector snakeml::math::vector::right		= vector{ +1.f, +0.f, +0.f };
inline const snakeml::math::vector snakeml::math::vector::left		= vector{ -1.f, +0.f, +0.f };
inline const snakeml::math::vector snakeml::math::vector::zero		= vector{ +0.f, +0.f, +0.f };

inline float snakeml::math::vector::length() const
{
	return sqrtf(x * x + y * y + z * z);
}

inline snakeml::math::vector snakeml::math::vector::getNormalized() const
{
	const float len = length();
	if (math::IsNearlyZero(len))
	{
		return math::vector::zero;
	}
	return vector { x / len, y / len, z / len };
}

inline void snakeml::math::vector::normalize()
{
	*this = getNormalized();
}

inline snakeml::math::vector snakeml::math::vector::cross(const vector& a) const
{
	return vector {y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x };
}

inline float snakeml::math::vector::dot(const vector& a) const
{
	return x * a.x + y * a.y + z * a.z;
}

inline snakeml::math::vector snakeml::math::vector::operator+(const vector& a) const
{
	return vector{ a.x + x, a.y + y, a.z + z };
}

inline void snakeml::math::vector::operator+=(const vector& a)
{
	x += a.x; y += a.y; z += a.z;
}

inline snakeml::math::vector snakeml::math::vector::operator-(const vector& a) const
{
	return vector{ x - a.x, y - a.y, z - a.z };
}


inline void snakeml::math::vector::operator-=(const snakeml::math::vector& a)
{
	x -= a.x; y -= a.y; z -= a.z;
}

inline snakeml::math::vector snakeml::math::vector::operator-() const
{
	return vector {-x, -y, -z};
}

inline snakeml::math::vector snakeml::math::vector::operator*(float scalar) const
{
	return vector { x * scalar, y * scalar, z * scalar };
}

inline void snakeml::math::vector::operator*=(float scalar)
{
	x *= scalar; y *= scalar; z *= scalar;
}

inline snakeml::math::vector snakeml::math::vector::operator/(float scalar) const
{
	return vector {x / scalar, y / scalar, z / scalar};
}

inline void snakeml::math::vector::operator/=(float scalar)
{
	x /= scalar; y /= scalar; z /= scalar;
}
