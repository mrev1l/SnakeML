#include "matrix.h"
#pragma once

inline const snakeml::math::matrix snakeml::math::matrix::identity =
{
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
};

inline snakeml::math::matrix::matrix()
{
	*this = identity;
}

inline snakeml::math::matrix::matrix(
	float _00, float _01, float _02, float _03,
	float _10, float _11, float _12, float _13,
	float _20, float _21, float _22, float _23,
	float _30, float _31, float _32, float _33
)
{
	m[0][0] = _00; m[0][1] = _01; m[0][2] = _02; m[0][3] = _03;
	m[1][0] = _10; m[1][1] = _11; m[1][2] = _12; m[1][3] = _13;
	m[2][0] = _20; m[2][1] = _21; m[2][2] = _22; m[2][3] = _23;
	m[3][0] = _30; m[3][1] = _31; m[3][2] = _32; m[3][3] = _33;
}


inline snakeml::math::matrix::matrix(const matrix& _m)
{
	copy(_m);
}

inline snakeml::math::matrix::matrix(matrix&& _m)
{
	std::swap(m, _m.m);
}

inline snakeml::math::matrix snakeml::math::matrix::inverse() const
{
	matrix augmented = *this;
	matrix inverse = identity;

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

inline snakeml::math::matrix& snakeml::math::matrix::operator=(const matrix& _m)
{
	copy(_m);
	return *this;
}

inline snakeml::math::matrix& snakeml::math::matrix::operator=(matrix&& _m)
{
	std::swap(m, _m.m);
	return *this;
}

inline snakeml::math::matrix snakeml::math::matrix::operator+(const matrix& _m) const
{
	return matrix(
		m[0][0] + _m.m[0][0], m[0][1] + _m.m[0][1], m[0][2] + _m.m[0][2], m[0][3] + _m.m[0][3],
		m[1][0] + _m.m[1][0], m[1][1] + _m.m[1][1], m[1][2] + _m.m[1][2], m[1][3] + _m.m[1][3],
		m[2][0] + _m.m[2][0], m[2][1] + _m.m[2][1], m[2][2] + _m.m[2][2], m[2][3] + _m.m[2][3],
		m[3][0] + _m.m[3][0], m[3][1] + _m.m[3][1], m[3][2] + _m.m[3][2], m[3][3] + _m.m[3][3]);
}

inline void snakeml::math::matrix::operator+=(const matrix& _m)
{
	*this = (*this + _m);
}

inline snakeml::math::matrix snakeml::math::matrix::operator-(const matrix& _m) const
{
	return matrix(
		m[0][0] - _m.m[0][0], m[0][1] - _m.m[0][1], m[0][2] - _m.m[0][2], m[0][3] - _m.m[0][3],
		m[1][0] - _m.m[1][0], m[1][1] - _m.m[1][1], m[1][2] - _m.m[1][2], m[1][3] - _m.m[1][3],
		m[2][0] - _m.m[2][0], m[2][1] - _m.m[2][1], m[2][2] - _m.m[2][2], m[2][3] - _m.m[2][3],
		m[3][0] - _m.m[3][0], m[3][1] - _m.m[3][1], m[3][2] - _m.m[3][2], m[3][3] - _m.m[3][3]);
}

inline void snakeml::math::matrix::operator-=(const matrix& _m)
{
	*this = (*this - _m);
}

inline snakeml::math::matrix snakeml::math::matrix::operator-() const
{
	return *this * -1.f;
}

inline snakeml::math::matrix snakeml::math::matrix::operator*(float scalar) const
{
	return matrix(
		m[0][0] * scalar, m[0][1] * scalar, m[0][2] * scalar, m[0][3] * scalar,
		m[1][0] * scalar, m[1][1] * scalar, m[1][2] * scalar, m[1][3] * scalar,
		m[2][0] * scalar, m[2][1] * scalar, m[2][2] * scalar, m[2][3] * scalar,
		m[3][0] * scalar, m[3][1] * scalar, m[3][2] * scalar, m[3][3] * scalar);
}

inline snakeml::math::matrix snakeml::math::matrix::operator*(const matrix& _m) const
{
	matrix res;
	for (size_t i = 0u; i < 16u; ++i)
	{
		res.m[i / 4u][i % 4u] = m[i / 4u][0] * _m.m[0][i % 4u] + m[i / 4u][1] * _m.m[1][i % 4u] +
			m[i / 4u][2] * _m.m[2][i % 4u] + m[i / 4u][3] * _m.m[3][i % 4u];
	}
	return res;
}

inline void snakeml::math::matrix::operator*=(const matrix& _m)
{
	*this = *this * _m;
}

inline snakeml::math::matrix snakeml::math::matrix::operator/(float scalar) const
{
	return *this * (1.f / scalar);
}

inline void snakeml::math::matrix::copy(const matrix& _m)
{
	for (size_t i = 0 ; i < 4; ++i)
	{
		const float* sourceBegin = &_m.m[i][0];
		const float* sourceEnd = &_m.m[i][0] + 4;
		float* destEnd = &m[i][0];

		std::copy(sourceBegin, sourceEnd, destEnd);
	}
}

inline snakeml::math::matrix ScaleMatrix(snakeml::math::vector scale)
{
	snakeml::math::matrix scaleMatrix;
	scaleMatrix.m[0][0] = scale.x;
	scaleMatrix.m[1][1] = scale.y;
	scaleMatrix.m[2][2] = scale.z;
	return scaleMatrix;
}

inline snakeml::math::matrix ScaleMatrix(float x, float y, float z)
{
	snakeml::math::vector scale(x, y, z);
	return ScaleMatrix(scale);
}

inline snakeml::math::matrix TranslationMatrix(snakeml::math::vector translation)
{
	snakeml::math::matrix scaleMatrix;
	scaleMatrix.m[3][0] = translation.x;
	scaleMatrix.m[3][1] = translation.y;
	scaleMatrix.m[3][2] = translation.z;
	return scaleMatrix;
}

inline snakeml::math::matrix TranslationMatrix(float x, float y, float z)
{
	snakeml::math::vector translation(x, y, z);
	return TranslationMatrix(translation);
}