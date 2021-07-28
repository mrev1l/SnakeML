#pragma once
namespace snakeml
{
namespace math
{

/*
* Separation Axis Theorem
*/
class SAT
{
public:
	static bool TestIntersection(const std::vector<math::vector>& aVertices, const std::vector<math::vector>& bVertices);

private:
	static bool IsAxisPresent(const math::vector& axisCandidate, std::vector<math::vector>& axises);
	static void BuildAxises(const std::vector<math::vector>& vertices, std::vector<math::vector>& _outAxises);
	static void ProjectPolygon(const math::vector& axis, const std::vector<math::vector>& vertices, float& _outProjectionMin, float& _outProjectionMax);
};

}
}