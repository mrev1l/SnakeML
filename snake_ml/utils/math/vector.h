#pragma once

namespace snakeml
{
namespace math
{

class vector
{
public:
	vector() = default;
	vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	~vector() = default;

	float length() const;
	vector getNormalized() const;
	void normalize();
	vector cross(const vector& a) const;
	float dot(const vector& a) const;

	vector operator+(const vector& a) const;
	void operator+=(const vector& a);
	
	vector operator-(const vector& a) const;
	void operator-=(const vector& a);
	vector operator-() const;

	vector operator*(float scalar) const;
	void operator*=(float scalar);

	vector operator/(float scalar) const;
	void operator/(float scalar);

	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	static const vector forward;
	static const vector backward;
	static const vector up;
	static const vector down;
	static const vector right;
	static const vector left;
	static const vector zero;
};

#include "vector.inl"

}
}