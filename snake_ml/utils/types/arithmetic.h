#pragma once

namespace snakeml
{
// TODO rename file
#define declare_type2(type) \
struct type##2 \
{ \
	type x, y; \
} 

#define declare_type3(type) \
struct type##3 \
{ \
	type x, y, z; \
}

#define declare_type4(type) \
struct type##3 \
{ \
	type x, y, z, w; \
}

declare_type2(uint32_t);
declare_type2(float);

declare_type3(float);

// TODO : Move to algorithm space
struct AABB
{
	vector min, max;

	static bool TestIntersection_AABB_AABB(const AABB& a, const AABB& b);
	static AABB GenerateAABB(const vector& dimensions, const vector& position, const vector& rotation);
};

}
