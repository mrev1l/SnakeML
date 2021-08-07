// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "arithmetic.h"

namespace snakeml
{
namespace types
{

bool AABB::TestIntersection_AABB_AABB(const AABB& a, const AABB& b)
{
	const bool isABehindB = a.min.x >= b.max.x;
	const bool isABeforeB = a.max.x <= b.min.x;
	const bool isAAboveA = a.min.y >= b.max.y;
	const bool isABelowB = a.max.y <= b.min.y;

	return !(isABehindB || isABeforeB || isAAboveA || isABelowB);
}

AABB AABB::GenerateAABB(const math::vector& dimensions, const math::vector& position, const math::vector& rotation)
{
	// Generate Bounding Box
	std::array<math::vector, 4> boundingBox =
	{
		math::vector{-dimensions.x / 2.f, -dimensions.y / 2.f, 0.f},
		math::vector{-dimensions.x / 2.f, +dimensions.y / 2.f, 0.f},
		math::vector{+dimensions.x / 2.f, -dimensions.y / 2.f, 0.f},
		math::vector{+dimensions.x / 2.f, +dimensions.y / 2.f, 0.f}
	};

	// Calc transform matrix for Bounding Box vertices
	const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(rotation.y), math::ConvertToRadians(rotation.x), math::ConvertToRadians(rotation.z));
	const math::matrix translationMatrix = math::TranslationMatrix(position.x, position.y, position.z);
	const math::matrix transformMatrix = rotationMatrix * translationMatrix;

	// Transform Bounding Box vertices
	for (auto& vertex : boundingBox)
	{
		vertex = transformMatrix * vertex;
	}

	// Construct AABB
	AABB result = { {FLT_MAX, FLT_MAX, 0.f}, {-FLT_MAX, -FLT_MAX, 0.f} };
	for (auto vertex : boundingBox)
	{
		result.min.x = std::min(result.min.x, vertex.x);
		result.min.y = std::min(result.min.y, vertex.y);
		result.max.x = std::max(result.max.x, vertex.x);
		result.max.y = std::max(result.max.y, vertex.y);
	}

	return result;
}

}
}