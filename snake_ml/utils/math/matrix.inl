#include "matrix.h"
#pragma once

template<typename T>
const snakeml::math::matrix<T> snakeml::math::matrix<T>::identity =
{
	static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0),
	static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0),
	static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0),
	static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)
};

template<typename T>
snakeml::math::matrix<T>::matrix()
{
	*this = identity;
}

template<typename T>
snakeml::math::matrix<T>::matrix(
	T _00, T _01, T _02, T _03,
	T _10, T _11, T _12, T _13,
	T _20, T _21, T _22, T _23,
	T _30, T _31, T _32, T _33
)
{
	m[0][0] = _00; m[0][1] = _01; m[0][2] = _02; m[0][3] = _03;
	m[1][0] = _10; m[1][1] = _11; m[1][2] = _12; m[1][3] = _13;
	m[2][0] = _20; m[2][1] = _21; m[2][2] = _22; m[2][3] = _23;
	m[3][0] = _30; m[3][1] = _31; m[3][2] = _32; m[3][3] = _33;
}


template<typename T>
snakeml::math::matrix<T>::matrix(const matrix<T>& _m)
{
	copy(_m);
}

template<typename T>
snakeml::math::matrix<T>::matrix(matrix<T>&& _m)
{
	std::swap(m, _m.m);
}

template<typename T>
inline snakeml::math::matrix<T> snakeml::math::matrix<T>::inverse() const
{
	matrix<T> augmented = *this;
	matrix<T> inverse = identity;

	auto& aug = augmented.m;
	auto& inv = inverse.m;

	for (size_t i = 0u; i < 4u; ++i)
	{
		ASSERT(aug[i][i] != 0.f, "Cannot inverse matrix");

		for (size_t j = 0u; j < 4u; ++j)
		{
			if (i != j)
			{
				float ratio = aug[j][i] / aug[i][i];
				for (size_t k = 0u; k < 4u; ++k)
				{
					aug[j][k] = aug[j][k] - ratio * aug[i][k];
					inv[j][k] = inv[j][k] - ratio * inv[i][k];
				}
			}
		}
	}

	for (size_t i = 0u; i < 4u; ++i)
	{
		for (size_t j = 0u; j < 4u; ++j)
		{
			inv[i][j] /= aug[i][i];
		}
	}

	return inverse;
}

template<typename T>
inline snakeml::math::matrix<T>& snakeml::math::matrix<T>::operator=(const matrix& _m)
{
	copy(_m);
	return *this;
}

template<typename T>
inline snakeml::math::matrix<T>& snakeml::math::matrix<T>::operator=(matrix&& _m)
{
	std::swap(m, _m.m);
	return *this;
}

template<typename T>
inline snakeml::math::matrix<T> snakeml::math::matrix<T>::operator+(const matrix<T>& _m) const
{
	return matrix<T>(
		m[0][0] + _m.m[0][0], m[0][1] + _m.m[0][1], m[0][2] + _m.m[0][2], m[0][3] + _m.m[0][3],
		m[1][0] + _m.m[1][0], m[1][1] + _m.m[1][1], m[1][2] + _m.m[1][2], m[1][3] + _m.m[1][3],
		m[2][0] + _m.m[2][0], m[2][1] + _m.m[2][1], m[2][2] + _m.m[2][2], m[2][3] + _m.m[2][3],
		m[3][0] + _m.m[3][0], m[3][1] + _m.m[3][1], m[3][2] + _m.m[3][2], m[3][3] + _m.m[3][3]);
}

template<typename T>
inline void snakeml::math::matrix<T>::operator+=(const matrix<T>& _m)
{
	*this = (*this + _m);
}

template<typename T>
inline snakeml::math::matrix<T> snakeml::math::matrix<T>::operator-(const matrix<T>& _m) const
{
	return matrix<T>(
		m[0][0] - _m.m[0][0], m[0][1] - _m.m[0][1], m[0][2] - _m.m[0][2], m[0][3] - _m.m[0][3],
		m[1][0] - _m.m[1][0], m[1][1] - _m.m[1][1], m[1][2] - _m.m[1][2], m[1][3] - _m.m[1][3],
		m[2][0] - _m.m[2][0], m[2][1] - _m.m[2][1], m[2][2] - _m.m[2][2], m[2][3] - _m.m[2][3],
		m[3][0] - _m.m[3][0], m[3][1] - _m.m[3][1], m[3][2] - _m.m[3][2], m[3][3] - _m.m[3][3]);
}

template<typename T>
inline void snakeml::math::matrix<T>::operator-=(const matrix<T>& _m)
{
	*this = (*this - _m);
}

template<typename T>
inline snakeml::math::matrix<T> snakeml::math::matrix<T>::operator-() const
{
	return *this * -1.f;
}

template<typename T>
inline snakeml::math::matrix<T> snakeml::math::matrix<T>::operator*(float scalar) const
{
	return matrix<T>(
		m[0][0] * scalar, m[0][1] * scalar, m[0][2] * scalar, m[0][3] * scalar,
		m[1][0] * scalar, m[1][1] * scalar, m[1][2] * scalar, m[1][3] * scalar,
		m[2][0] * scalar, m[2][1] * scalar, m[2][2] * scalar, m[2][3] * scalar,
		m[3][0] * scalar, m[3][1] * scalar, m[3][2] * scalar, m[3][3] * scalar);
}

template<typename T>
inline snakeml::math::matrix<T> snakeml::math::matrix<T>::operator*(const matrix<T>& _m) const
{
	matrix<T> res;
	for (size_t i = 0u; i < 16u; ++i)
	{
		res.m[i / 4u][i % 4u] = m[i / 4u][0] * _m.m[0][i % 4u] + m[i / 4u][1] * _m.m[1][i % 4u] +
			m[i / 4u][2] * _m.m[2][i % 4u] + m[i / 4u][3] * _m.m[3][i % 4u];
	}
	return res;
}

template<typename T>
inline void snakeml::math::matrix<T>::operator*=(const matrix<T>& _m)
{
	*this = *this * _m;
}

template<typename T>
inline snakeml::math::matrix<T> snakeml::math::matrix<T>::operator/(float scalar) const
{
	return *this * (1.f / scalar);
}

template<typename T>
inline void snakeml::math::matrix<T>::copy(const matrix& _m)
{
	for (size_t i = 0 ; i < 4; ++i)
	{
		const T* sourceBegin = &_m.m[i][0];
		const T* sourceEnd = &_m.m[i][0] + 4;
		T* destEnd = &m[i][0];

		std::copy(sourceBegin, sourceEnd, destEnd);
	}
}
