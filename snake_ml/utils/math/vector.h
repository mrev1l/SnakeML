#pragma once

namespace snakeml
{
namespace math
{

template<typename T>
class vector
{
public:
	vector() = default;
	vector(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
	~vector() = default;

	float length() const;
	vector<T> getNormalized() const;
	void normalize();
	vector<T> cross(const vector<T>& a) const;
	float dot(const vector<T>& a) const;

	vector<T> operator+(const vector<T>& a) const;
	void operator+=(const vector<T>& a);
	
	vector<T> operator-(const vector<T>& a) const;
	void operator-=(const vector<T>& a);
	vector<T> operator-();

	vector<T> operator*(T scalar) const;
	void operator*=(T scalar);

	vector<T> operator/(T scalar) const;
	void operator/(T scalar);

	T x = static_cast<T>(0);
	T y = static_cast<T>(0);
	T z = static_cast<T>(0);

	static const vector<T> forward	= vector<T> { static_cast<T>(+0), static_cast<T>(+0), static_cast<T>(+1) };
	static const vector<T> backward	= vector<T> { static_cast<T>(+0), static_cast<T>(+0), static_cast<T>(-1) };
	static const vector<T> up		= vector<T> { static_cast<T>(+0), static_cast<T>(+1), static_cast<T>(+0) };
	static const vector<T> down		= vector<T> { static_cast<T>(+0), static_cast<T>(-1), static_cast<T>(+0) };
	static const vector<T> right	= vector<T> { static_cast<T>(+1), static_cast<T>(+0), static_cast<T>(+0) };
	static const vector<T> left		= vector<T> { static_cast<T>(-1), static_cast<T>(+0), static_cast<T>(+0) };
	static const vector<T> zero		= vector<T> { static_cast<T>(+0), static_cast<T>(+0), static_cast<T>(+0) };
};

#include "vector.inl"

using vectorf = vector<float>;

}
}