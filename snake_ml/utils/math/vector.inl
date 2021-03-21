#pragma once

template<typename T>
inline float snakeml::math::vector<T>::length() const
{
	return sqrtf(x * x + y * y + z * z);
}

template<typename T>
inline snakeml::math::vector<T> snakeml::math::vector<T>::getNormalized() const
{
	const float len = length();
	return vector<T> { x / len, y / len, z / len };
}

template<typename T>
inline void snakeml::math::vector<T>::normalize()
{
	*this = getNormalized();
}

template<typename T>
inline snakeml::math::vector<T> snakeml::math::vector<T>::cross(const vector<T>& a) const
{
	return vector<T> {y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x };
}

template<typename T>
inline float snakeml::math::vector<T>::dot(const vector<T>& a) const
{
	return x * a.x + y * a.y + z * a.z;
}

template<typename T>
inline snakeml::math::vector<T> snakeml::math::vector<T>::operator+(const vector<T>& a) const
{
	return vector<T>{ a.x + x, a.y + y, a.z + z };
}

template<typename T>
inline void snakeml::math::vector<T>::operator+=(const vector<T>& a)
{
	x += a.x; y += a.y; z += z.z;
}

template<typename T>
inline snakeml::math::vector<T> snakeml::math::vector<T>::operator-(const vector<T>& a) const
{
	return vector<T>{ x - a.x, y - a.y, z - a.z };
}


template<typename T>
inline void snakeml::math::vector<T>::operator-=(const snakeml::math::vector<T>& a)
{
	x -= a.x; y -= a.y; z -= a.z;
}

template<typename T>
inline snakeml::math::vector<T> snakeml::math::vector<T>::operator-()
{
	return vector<T> {-x, -y, -z};
}

template<typename T>
inline snakeml::math::vector<T> snakeml::math::vector<T>::operator*(T scalar) const
{
	return vector<T> { x * scalar, y * scalar, z * scalar };
}

template<typename T>
inline void snakeml::math::vector<T>::operator*=(T scalar)
{
	x *= scalar; y *= scalar; z *= scalar;
}

template<typename T>
inline snakeml::math::vector<T> snakeml::math::vector<T>::operator/(T scalar) const
{
	return vector<T> {x / scalar, y / scalar, z / scalar};
}

template<typename T>
inline void snakeml::math::vector<T>::operator/(T scalar)
{
	x /= scalar; y /= scalar; z /= scalar;
}
