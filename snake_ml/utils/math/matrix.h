#pragma once
#include "vector.h"

namespace snakeml
{
namespace math
{

class matrix
{
public:
	matrix();
	matrix(
		float _00, float _01, float _02, float _03,
		float _10, float _11, float _12, float _13,
		float _20, float _21, float _22, float _23,
		float _30, float _31, float _32, float _33);
	matrix(const matrix& _m);
	matrix(matrix&& _m);
	~matrix() = default;

	matrix inverse() const;

	matrix& operator=(const matrix& _m);
	matrix& operator=(matrix&& _m);

	matrix operator+(const matrix& _m) const;
	void operator+=(const matrix& _m);

	matrix operator-(const matrix& _m) const;
	void operator-=(const matrix& _m);
	matrix operator-() const;

	matrix operator*(float scalar) const;
	matrix operator*(const matrix& _m) const;
	void operator*=(const matrix& _m);

	matrix operator/(float scalar) const;

	float m[4][4];

	static const matrix identity;

private:
	void copy(const matrix& _m);
};

inline matrix ScaleMatrix(vector scale);
inline matrix ScaleMatrix(float x, float y, float z);

inline matrix TranslationMatrix(vector translation);
inline matrix TranslationMatrix(float x, float y, float z);

inline matrix RotationYawMatrix(float yawRad);
inline matrix RotationPitchMatrix(float pitchRad);
inline matrix RotationRollMatrix(float rollRad);
inline matrix RotationMatrix(float yawRad, float pitchRad, float rollRad);

#include "matrix.inl"

}
}