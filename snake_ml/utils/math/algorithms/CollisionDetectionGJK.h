#pragma once
namespace snakeml
{

class GJK
{
public:
	struct Intersection
	{
		bool areIntersecting = false;
		float penetrationDepth = 0.f;
		vector penetrationVector = vector::zero;
	};

	static Intersection TestIntersection(const std::vector<vector>& aVertices, const std::vector<vector>& bVertices, const vector& aCenter, const vector& bCenter);

private:
	/*
	* GJK
	* https://www.youtube.com/watch?v=ajv46BSqcK4
	*/

	struct SupportPoint
	{
		SupportPoint(const vector& p, float d) : point(p), distance(d) {}
		vector	point;
		float			distance;
	};

	static void FindSupportPoint(const vector& supportDirection, const std::vector<vector>& vertices, SupportPoint& _outSupportPoint);
	static vector SupportFunction(const std::vector<vector>& aVertices, const std::vector<vector>& bVertices, const vector& direction);

	static bool HandleSimplex(std::vector<vector>& simplex, vector& _outDirection);
	static bool HandleSimplex_LineCase(const std::vector<vector>& simplex, vector& _outDirection);
	static bool HandleSimplex_TriangleCase(std::vector<vector>& simplex, vector& _outDirection);

	/*
	* Expanding Polytope algorithm
	* https://www.youtube.com/watch?v=0XQ2FSz3EK8
	*/
	static Intersection EPA(std::vector<vector>& simplex, const std::vector<vector>& aVertices, const std::vector<vector>& bVertices);
};

}
