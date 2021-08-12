// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "CollisionDetectionSAT.h"

namespace snakeml
{

bool SAT::TestIntersection(const std::vector<vector>& aVertices, const std::vector<vector>& bVertices)
{
	std::vector<vector> axises;

	BuildAxises(aVertices, axises);
	BuildAxises(bVertices, axises);

	for (const vector& axis : axises)
	{
		float aProjectionMin = FLT_MAX, aProjectionMax = -FLT_MAX, bProjectionMin = FLT_MAX, bProjectionMax = -FLT_MAX;

		ProjectPolygon(axis, aVertices, aProjectionMin, aProjectionMax);
		ProjectPolygon(axis, bVertices, bProjectionMin, bProjectionMax);

		if (
			aProjectionMax <= bProjectionMin || // a is before b
			aProjectionMin >= bProjectionMax)	// a is after b
		{
			return false;
		}
	}

	return true; // no separating axis found
}

bool SAT::IsAxisPresent(const vector& axisCandidate, std::vector<vector>& axises)
{
	const float axisDirectionEqualityThreshold = cosf(ConvertToRadians(1.f));
	const auto AreTheSameAxis = [axisCandidate, axisDirectionEqualityThreshold](const vector& a) -> bool
	{
		const float dot = axisCandidate.dot(a);
		return IsNearlyEqual(std::abs(dot), 1.f, axisDirectionEqualityThreshold);
	};

	return std::find_if(axises.begin(), axises.end(), AreTheSameAxis) != axises.end();
}

void SAT::BuildAxises(const std::vector<vector>& vertices, std::vector<vector>& _outAxises)
{
	for (size_t i = 0u; i < vertices.size(); ++i)
	{
		const size_t j = (i + 1) % vertices.size();
		const vector edge = perpendicular2d(vertices[j] - vertices[i]);
		const vector axisCandidate = edge.getNormalized();

		// don't allow the same axis to occur multiple times
		if (!IsAxisPresent(axisCandidate, _outAxises))
		{
			_outAxises.push_back(axisCandidate);
		}
	}
}

void SAT::ProjectPolygon(const vector& axis, const std::vector<vector>& vertices, float& _outProjectionMin, float& _outProjectionMax)
{
	for (const vector& vertex : vertices)
	{
		const float projection = vertex.dot(axis);
		_outProjectionMin = std::min(_outProjectionMin, projection);
		_outProjectionMax = std::max(_outProjectionMax, projection);
	}
}

}
