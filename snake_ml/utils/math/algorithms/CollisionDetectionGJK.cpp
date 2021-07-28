// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "CollisionDetectionGJK.h"

namespace snakeml
{
namespace math
{

GJK::Intersection GJK::TestIntersection(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices, const math::vector& aCenter, const math::vector& bCenter)
{
	math::vector direction = (bCenter - aCenter).getNormalized();
	std::vector<math::vector> simplex = { SupportFunction(aVertices, bVertices, direction) };
	direction = (math::vector::zero - simplex[0]).getNormalized();
	while (true)
	{
		math::vector supportPoint = SupportFunction(aVertices, bVertices, direction);
		if (supportPoint.dot(direction) < 0.f)
		{
			return Intersection{};
		}
		simplex.push_back(supportPoint);
		if (HandleSimplex(simplex, direction))
		{
			break;
		}
	}

	return EPA(simplex, aVertices, bVertices);
}

void GJK::FindSupportPoint(const math::vector& supportDirection, const std::vector<math::vector>& vertices, SupportPoint& _outSupportPoint)
{
	for (const math::vector& v : vertices)
	{
		const float dot = supportDirection.dot(v);
		if (dot > _outSupportPoint.distance)
		{
			_outSupportPoint = SupportPoint{ v, dot };
		}
	}
}

math::vector GJK::SupportFunction(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices, const math::vector& direction)
{
	SupportPoint furthestPointA = { math::vector::zero, -FLT_MAX }, furthestPointB = { math::vector::zero, -FLT_MAX };

	FindSupportPoint(direction, aVertices, furthestPointA);
	FindSupportPoint(-direction, bVertices, furthestPointB);

	return furthestPointA.point - furthestPointB.point;
}

bool GJK::HandleSimplex(std::vector<math::vector>& simplex, math::vector& _outDirection)
{
	if (simplex.size() == 2)
	{
		return HandleSimplex_LineCase(simplex, _outDirection);
	}
	ASSERT(simplex.size() == 3, "Error in GJK");
	return HandleSimplex_TriangleCase(simplex, _outDirection);
}

bool GJK::HandleSimplex_LineCase(const std::vector<math::vector>& simplex, math::vector& _outDirection)
{
	const math::vector& a = simplex[1];
	const math::vector& b = simplex[0];
	const math::vector ab = b - a;
	const math::vector ao = math::vector::zero - a;

	const math::vector triProduct = triple_product(ab, ao, ab);
	const math::vector abPerp = math::IsNearlyZero(triProduct.length()) ? perpendicular2d(ab) : triProduct;

	_outDirection = abPerp.getNormalized();
	return false;
}

bool GJK::HandleSimplex_TriangleCase(std::vector<math::vector>& simplex, math::vector& _outDirection)
{
	const math::vector& a = simplex[2];
	const math::vector& b = simplex[1];
	const math::vector& c = simplex[0];
	const math::vector ab = b - a;
	const math::vector ac = c - a;
	const math::vector ao = math::vector::zero - a;
	const math::vector abTriProduct = triple_product(ac, ab, ab);
	const math::vector acTriProduct = triple_product(ab, ac, ac);
	const math::vector abPerp = math::IsNearlyZero(abTriProduct.length()) ? perpendicular2d(ab) : abTriProduct;
	const math::vector acPerp = math::IsNearlyZero(acTriProduct.length()) ? perpendicular2d(ac) : acTriProduct;
	
	if (abPerp.dot(ao) > 0.f)
	{
		simplex.erase(simplex.begin());
		_outDirection = abPerp.getNormalized();
		return false;
	}
	else if (acPerp.dot(ao) > 0.f)
	{
		simplex.erase(simplex.begin() + 1);
		_outDirection = acPerp.getNormalized();
		return false;
	}
	return true;
}

GJK::Intersection GJK::EPA(std::vector<math::vector>& simplex, const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices)
{
	float minDistance = FLT_MAX;
	size_t minIdx = 0u;
	math::vector minNormal = math::vector::zero;

	while (math::IsNearlyEqual(minDistance, FLT_MAX))
	{
		for (size_t i = 0u; i < simplex.size(); ++i)
		{
			size_t j = (i + 1) % simplex.size();

			const math::vector edge = simplex[j] - simplex[i];
			math::vector edgeNormal = math::vector(edge.y, -edge.x, 0.f).getNormalized();
			float distance = edgeNormal.dot(simplex[i]);

			if (distance < 0)
			{
				edgeNormal *= -1.f;
				distance *= -1.f;
			}

			if (distance < minDistance)
			{
				minDistance = distance;
				minNormal = edgeNormal;
				minIdx = j;
			}
		}

		const math::vector supportPoint = SupportFunction(aVertices, bVertices, minNormal);
		const float supportDistance = minNormal.dot(supportPoint);

		if (std::abs(minDistance - supportDistance) > math::k_default_epsilon)
		{
			minDistance = FLT_MAX;
			simplex.insert(simplex.begin() + minIdx, supportPoint);
		}
	}

	return Intersection{ true, minDistance, minNormal };
}

}
}