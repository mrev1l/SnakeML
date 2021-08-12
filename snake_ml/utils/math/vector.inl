#pragma once

inline const snakeml::vector snakeml::vector::forward	= vector{ +0.f, +0.f, +1.f };
inline const snakeml::vector snakeml::vector::backward	= vector{ +0.f, +0.f, -1.f };
inline const snakeml::vector snakeml::vector::up		= vector{ +0.f, +1.f, +0.f };
inline const snakeml::vector snakeml::vector::down		= vector{ +0.f, -1.f, +0.f };
inline const snakeml::vector snakeml::vector::right		= vector{ +1.f, +0.f, +0.f };
inline const snakeml::vector snakeml::vector::left		= vector{ -1.f, +0.f, +0.f };
inline const snakeml::vector snakeml::vector::zero		= vector{ +0.f, +0.f, +0.f };

inline float snakeml::vector::length() const
{
	return sqrtf(x * x + y * y + z * z);
}

inline snakeml::vector snakeml::vector::getNormalized() const
{
	const float len = length();
	if (IsNearlyZero(len))
	{
		return vector::zero;
	}
	return vector { x / len, y / len, z / len };
}

inline void snakeml::vector::normalize()
{
	*this = getNormalized();
}

inline snakeml::vector snakeml::vector::cross(const vector& a) const
{
	return vector {y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x };
}

inline float snakeml::vector::dot(const vector& a) const
{
	return x * a.x + y * a.y + z * a.z;
}

inline snakeml::vector snakeml::vector::operator+(const vector& a) const
{
	return vector{ a.x + x, a.y + y, a.z + z };
}

inline void snakeml::vector::operator+=(const vector& a)
{
	x += a.x; y += a.y; z += a.z;
}

inline snakeml::vector snakeml::vector::operator-(const vector& a) const
{
	return vector{ x - a.x, y - a.y, z - a.z };
}


inline void snakeml::vector::operator-=(const snakeml::vector& a)
{
	x -= a.x; y -= a.y; z -= a.z;
}

inline snakeml::vector snakeml::vector::operator-() const
{
	return vector {-x, -y, -z};
}

inline snakeml::vector snakeml::vector::operator*(float scalar) const
{
	return vector { x * scalar, y * scalar, z * scalar };
}

inline void snakeml::vector::operator*=(float scalar)
{
	x *= scalar; y *= scalar; z *= scalar;
}

inline snakeml::vector snakeml::vector::operator/(float scalar) const
{
	return vector {x / scalar, y / scalar, z / scalar};
}

inline void snakeml::vector::operator/=(float scalar)
{
	x /= scalar; y /= scalar; z /= scalar;
}
