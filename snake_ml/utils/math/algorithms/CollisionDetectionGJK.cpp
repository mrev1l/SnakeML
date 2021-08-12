// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "CollisionDetectionGJK.h"

namespace snakeml
{

GJK::Intersection GJK::TestIntersection(const std::vector<vector>& aVertices, const std::vector<vector>& bVertices, const vector& aCenter, const vector& bCenter)
{
	vector direction = (bCenter - aCenter).getNormalized();
	std::vector<vector> simplex = { SupportFunction(aVertices, bVertices, direction) };
	direction = (vector::zero - simplex[0]).getNormalized();
	while (true)
	{
		vector supportPoint = SupportFunction(aVertices, bVertices, direction);
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

void GJK::FindSupportPoint(const vector& supportDirection, const std::vector<vector>& vertices, SupportPoint& _outSupportPoint)
{
	for (const vector& v : vertices)
	{
		const float dot = supportDirection.dot(v);
		if (dot > _outSupportPoint.distance)
		{
			_outSupportPoint = SupportPoint{ v, dot };
		}
	}
}

vector GJK::SupportFunction(const std::vector<vector>& aVertices, const std::vector<vector>& bVertices, const vector& direction)
{
	SupportPoint furthestPointA = { vector::zero, -FLT_MAX }, furthestPointB = { vector::zero, -FLT_MAX };

	FindSupportPoint(direction, aVertices, furthestPointA);
	FindSupportPoint(-direction, bVertices, furthestPointB);

	return furthestPointA.point - furthestPointB.point;
}

bool GJK::HandleSimplex(std::vector<vector>& simplex, vector& _outDirection)
{
	if (simplex.size() == 2)
	{
		return HandleSimplex_LineCase(simplex, _outDirection);
	}
	ASSERT(simplex.size() == 3, "Error in GJK");
	return HandleSimplex_TriangleCase(simplex, _outDirection);
}

bool GJK::HandleSimplex_LineCase(const std::vector<vector>& simplex, vector& _outDirection)
{
	const vector& a = simplex[1];
	const vector& b = simplex[0];
	const vector ab = b - a;
	const vector ao = vector::zero - a;

	const vector triProduct = triple_product(ab, ao, ab);
	const vector abPerp = IsNearlyZero(triProduct.length()) ? perpendicular2d(ab) : triProduct;

	_outDirection = abPerp.getNormalized();
	return false;
}

bool GJK::HandleSimplex_TriangleCase(std::vector<vector>& simplex, vector& _outDirection)
{
	const vector& a = simplex[2];
	const vector& b = simplex[1];
	const vector& c = simplex[0];
	const vector ab = b - a;
	const vector ac = c - a;
	const vector ao = vector::zero - a;
	const vector abTriProduct = triple_product(ac, ab, ab);
	const vector acTriProduct = triple_product(ab, ac, ac);
	const vector abPerp = IsNearlyZero(abTriProduct.length()) ? perpendicular2d(ab) : abTriProduct;
	const vector acPerp = IsNearlyZero(acTriProduct.length()) ? perpendicular2d(ac) : acTriProduct;
	
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

GJK::Intersection GJK::EPA(std::vector<vector>& simplex, const std::vector<vector>& aVertices, const std::vector<vector>& bVertices)
{
	float minDistance = FLT_MAX;
	size_t minIdx = 0u;
	vector minNormal = vector::zero;

	while (IsNearlyEqual(minDistance, FLT_MAX))
	{
		for (size_t i = 0u; i < simplex.size(); ++i)
		{
			size_t j = (i + 1) % simplex.size();

			const vector edge = simplex[j] - simplex[i];
			vector edgeNormal = vector(edge.y, -edge.x, 0.f).getNormalized();
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

		const vector supportPoint = SupportFunction(aVertices, bVertices, minNormal);
		const float supportDistance = minNormal.dot(supportPoint);

		if (std::abs(minDistance - supportDistance) > k_default_epsilon)
		{
			minDistance = FLT_MAX;
			simplex.insert(simplex.begin() + minIdx, supportPoint);
		}
	}

	return Intersection{ true, minDistance, minNormal };
}

}
