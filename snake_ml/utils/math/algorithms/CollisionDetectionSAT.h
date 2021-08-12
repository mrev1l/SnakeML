#pragma once
namespace snakeml
{

/*
* Separation Axis Theorem
*/
class SAT
{
public:
	static bool TestIntersection(const std::vector<vector>& aVertices, const std::vector<vector>& bVertices);

private:
	static bool IsAxisPresent(const vector& axisCandidate, std::vector<vector>& axises);
	static void BuildAxises(const std::vector<vector>& vertices, std::vector<vector>& _outAxises);
	static void ProjectPolygon(const vector& axis, const std::vector<vector>& vertices, float& _outProjectionMin, float& _outProjectionMax);
};

}
