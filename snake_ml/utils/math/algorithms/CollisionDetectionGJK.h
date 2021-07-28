#pragma once
namespace snakeml
{
namespace math
{

class GJK
{
public:
	struct Intersection
	{
		bool areIntersecting = false;
		float penetrationDepth = 0.f;
		math::vector penetrationVector = math::vector::zero;
	};

	static Intersection TestIntersection(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices, const math::vector& aCenter, const math::vector& bCenter);

private:
	/*
	* GJK
	* https://www.youtube.com/watch?v=ajv46BSqcK4
	*/

	struct SupportPoint
	{
		SupportPoint(const math::vector& p, float d) : point(p), distance(d) {}
		math::vector	point;
		float			distance;
	};

	static void FindSupportPoint(const math::vector& supportDirection, const std::vector<math::vector>& vertices, SupportPoint& _outSupportPoint);
	static math::vector SupportFunction(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices, const math::vector& direction);

	static bool HandleSimplex(std::vector<math::vector>& simplex, math::vector& _outDirection);
	static bool HandleSimplex_LineCase(const std::vector<math::vector>& simplex, math::vector& _outDirection);
	static bool HandleSimplex_TriangleCase(std::vector<math::vector>& simplex, math::vector& _outDirection);

	/*
	* Expanding Polytope algorithm
	* https://www.youtube.com/watch?v=0XQ2FSz3EK8
	*/
	static Intersection EPA(std::vector<math::vector>& simplex, const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices);
};

}
}