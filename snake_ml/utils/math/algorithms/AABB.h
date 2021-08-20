#pragma once

namespace snakeml
{

struct AABB
{
	vector min, max;

	static bool TestIntersection_AABB_AABB(const AABB& a, const AABB& b);
	static AABB GenerateAABB(const vector& dimensions, const vector& position, const vector& rotation);
};

}