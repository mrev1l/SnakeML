// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "WIP_System.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <chrono>

namespace snakeml
{
namespace system
{

class Capsule 
{
public:
	math::vector m_tip;
	math::vector m_base;
	float m_radius;
};

math::vector ClosestPointOnLineSegment(math::vector A, math::vector B, math::vector Point)
{
  math::vector AB = B - A;
  float t = (Point - A).dot(AB) / AB.lengthSq();
  return A + AB * std::clamp(t, 0.f, 1.f);
}

void CapsuleVCapsuleTest()
{
	Capsule CapsuleA = Capsule{ math::vector{-25.f, 100.f, 0.f}, math::vector{-25.f, -100.f, 0.f}, 50.f };
	Capsule CapsuleB = Capsule{ math::vector{25.f, 100.f, 0.f}, math::vector{25.f, -100.f, 0.f}, 50.f };

	// capsule A:
	math::vector a_Normal = (CapsuleA.m_tip - CapsuleA.m_base).getNormalized();
	math::vector a_LineEndOffset = a_Normal * CapsuleA.m_radius;
	math::vector a_A = CapsuleA.m_base + a_LineEndOffset;
	math::vector a_B = CapsuleA.m_tip - a_LineEndOffset;

	// capsule B:
	math::vector b_Normal = (CapsuleB.m_tip - CapsuleB.m_base).getNormalized();
	math::vector b_LineEndOffset = b_Normal * CapsuleB.m_radius;
	math::vector b_A = CapsuleB.m_base + b_LineEndOffset;
	math::vector b_B = CapsuleB.m_tip - b_LineEndOffset;

	// vectors between line endpoints:
	math::vector v0 = b_A - a_A;
	math::vector v1 = b_B - a_A;
	math::vector v2 = b_A - a_B;
	math::vector v3 = b_B - a_B;

	// squared distances:
	float d0 = v0.lengthSq();
	float d1 = v1.lengthSq();
	float d2 = v2.lengthSq();
	float d3 = v3.lengthSq();

	// select best potential endpoint on capsule A:
	math::vector bestA;
	if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
	{
		bestA = a_B;
	}
	else
	{
		bestA = a_A;
	}

	// select point on capsule B line segment nearest to best potential endpoint on A capsule:
	math::vector bestB = ClosestPointOnLineSegment(b_A, b_B, bestA);

	// now do the same for capsule A segment:
	bestA = ClosestPointOnLineSegment(a_A, a_B, bestB);

	math::vector penetration_normal = (bestA - bestB);
	float length = penetration_normal.length();
	penetration_normal.normalize();
	float penetration_depth = CapsuleA.m_radius + CapsuleB.m_radius - length;
	bool intersects = penetration_depth > 0;
}

void CapsuleVTriangleTest()
{
	Capsule CapsuleA = Capsule{ math::vector{-100.f, -100.f, 0.f}, math::vector{-100.f, -100.f, 0.f}, 50.f };
	math::vector p0(200.f, 100.f, 0.f), p1(175.f, 0.f, 0.f), p2(225.f, 0.f, 0.f); // triangle corners

	// Compute capsule line endpoints A, B like before in capsule-capsule case:
	math::vector CapsuleAxis = CapsuleA.m_tip - CapsuleA.m_base;
	math::vector CapsuleNormal = CapsuleAxis.lengthSq() > 0.f ? (CapsuleA.m_tip - CapsuleA.m_base).getNormalized() : math::vector::zero;
	math::vector LineEndOffset = CapsuleNormal * CapsuleA.m_radius;
	math::vector A = CapsuleA.m_base + LineEndOffset;
	math::vector B = CapsuleA.m_tip - LineEndOffset;

	// Compute plane normal
	math::vector N = ((p1 - p0).cross(p2 - p0)).getNormalized();//normalize(cross(p1 – p0, p2 – p0));

	// Then for each triangle, ray-plane intersection:
	float a = N.dot(p0 - CapsuleA.m_base);
	float b = abs(N.dot(CapsuleNormal));
	float t = N.dot(p0 - CapsuleA.m_base) / abs(N.dot(CapsuleNormal));//dot(N, (p0 - CapsuleA.m_base) / abs(dot(N, CapsuleNormal)));
	math::vector line_plane_intersection = CapsuleA.m_base + CapsuleNormal * t;

	// p = l0 + l * d
	float d = (p0 - CapsuleA.m_base).dot(N) / (CapsuleNormal.dot(N));
	math::vector point_of_intersection = CapsuleA.m_base + CapsuleNormal * d;

	/// <find closest point on triangle to line_plane_intersection>
	// Determine whether line_plane_intersection is inside all triangle edges: 
	math::vector c0 = (line_plane_intersection - p0).cross(p1 - p0);
	math::vector c1 = (line_plane_intersection - p1).cross(p2 - p1);
	math::vector c2 = (line_plane_intersection - p2).cross(p0 - p2);
	bool inside = c0.dot(N) <= 0 && c1.dot(N) <= 0 && c2.dot(N) <= 0;

	math::vector reference_point;
	if (inside)
	{
		reference_point = line_plane_intersection;
	}
	else
	{
		// Edge 1:
		math::vector point1 = ClosestPointOnLineSegment(p0, p1, line_plane_intersection);
		math::vector v1 = line_plane_intersection - point1;
		float distsq = v1.lengthSq();// dot(v1, v1);
		float best_dist = distsq;
		reference_point = point1;

		// Edge 2:
		math::vector point2 = ClosestPointOnLineSegment(p1, p2, line_plane_intersection);
		math::vector v2 = line_plane_intersection - point2;
		distsq = v2.lengthSq();// dot(vec2, vec2);
		if (distsq < best_dist)
		{
			reference_point = point2;
			best_dist = distsq;
		}

		// Edge 3:
		math::vector point3 = ClosestPointOnLineSegment(p2, p0, line_plane_intersection);
		math::vector v3 = line_plane_intersection - point3;
		distsq = v3.lengthSq();// dot(v3, v3);
		if (distsq < best_dist)
		{
			reference_point = point3;
			best_dist = distsq;
		}
	}
	/// </find closest point on triangle to line_plane_intersection>

	// The center of the best sphere candidate:
	math::vector center = ClosestPointOnLineSegment(A, B, reference_point);

	float radius = CapsuleA.m_radius;

	// L = { center.x + N.x * t ; center.y + N.y * t ; center.z + N.z * t }

	float t1 = -(N.x * center.x + N.y * center.y + N.z * center.z + N.dot(p1)) / (N.lengthSq());

	math::vector intersection_center = { center.x + N.x * t1, center.y + N.y * t1, center.z + N.z * t1 };
	float d1 = (center - intersection_center).length();
	float r = sqrtf(radius * radius - d1 * d1);

	float dist = (center - p0).dot(N);

	int stop = 7;
}

void SphereVPlaneTest()
{
	//math::vector p0(0.f, 0.f, 0.f), p1(10.f, 0.f, 0.f), p2(0.f, 5.f, 0.f); // triangle corners
	math::vector p0(200.f, 100.f, 0.f), p1(175.f, 0.f, 0.f), p2(225.f, 0.f, 0.f); // triangle corners
	//math::vector center(5.f, 1.f, 0.f); // sphere center
	math::vector center(-100.f, 0.f, 0.f); // sphere center
	//float radius = 1.f;
	float radius = 50.f;
	math::vector N = ((p1 - p0).cross(p2 - p0)).getNormalized();

	// L = { center.x + N.x * t ; center.y + N.y * t ; center.z + N.z * t }

	float t = -(N.x * center.x + N.y * center.y + N.z * center.z + N.dot(p1)) / (N.lengthSq());

	math::vector intersection_center = { center.x + N.x * t, center.y + N.y * t, center.z + N.z * t };
	float d = (center - intersection_center).length();
	float r = sqrtf(radius * radius - d * d);

	float dist = (center - p0).dot(N);

	int stop = 9;
}

void WIP_System::Execute()
{
	//CapsuleVCapsuleTest();

	CapsuleVTriangleTest(); 

	//SphereVPlaneTest();

	int stop = 34;
}

}
}