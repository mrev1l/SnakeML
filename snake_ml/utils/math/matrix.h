#pragma once

namespace snakeml
{
namespace math
{

template<typename T>
class matrix
{
public:
	matrix();
	matrix(
		T _00, T _01, T _02, T _03,
		T _10, T _11, T _12, T _13,
		T _20, T _21, T _22, T _23,
		T _30, T _31, T _32, T _33);
	matrix(const matrix& _m);
	matrix(matrix&& _m);
	~matrix() = default;

	matrix<T> inverse() const;

	matrix<T>& operator=(const matrix& _m);
	matrix<T>& operator=(matrix&& _m);

	matrix<T> operator+(const matrix<T>& _m) const;
	void operator+=(const matrix<T>& _m);

	matrix<T> operator-(const matrix<T>& _m) const;
	void operator-=(const matrix<T>& _m);
	matrix<T> operator-() const;

	matrix<T> operator*(float scalar) const;
	matrix<T> operator*(const matrix<T>& _m) const;
	void operator*=(const matrix<T>& _m);

	matrix<T> operator/(float scalar) const;

	static const matrix<T> identity;

private:
	void copy(const matrix& _m);

	T m[4][4];
};

#include "matrix.inl"

using matrixf = matrix<float>;

}
}