#pragma once

#include "system/ecs/Entity.h"
#include "system/ecs/ISystem.h"

#include "utils/math/algorithms/CollisionDetectionGJK.h"
#include "utils/types/QuadTree.h"

namespace snakeml
{
namespace system
{

class PhysicsComponent;
class PhysicsComponentIterator;
class TransformComponent;

class PhysicsSystem : public ISystem
{
public:
	PhysicsSystem() : ISystem() {}

	void Update(double deltaTime) override;

private:
	using Polygon = std::vector<math::vector>;
	struct NarrowPhaseBody
	{
		NarrowPhaseBody(PhysicsComponent& physicsObj, Polygon&& poly) : physicsObject(physicsObj), polygon(std::move(poly)) { }
		PhysicsComponent& physicsObject;
		Polygon polygon;
	};
	struct NarrowPhasePair
	{
		NarrowPhasePair(NarrowPhaseBody&& _a, NarrowPhaseBody&& _b) : a(std::move(_a)), b(std::move(_b)) { }
		NarrowPhaseBody a;
		NarrowPhaseBody b;
	};

	void SimulatePhysics(PhysicsComponentIterator* bodiesIt, double dt);
	void SimulatePhysicsStep(PhysicsComponent& body, TransformComponent& transform, double dt);

	void UpdateAABBs(PhysicsComponentIterator* bodiesIt);
	void UpdateAABB(PhysicsComponent& body);

	void PopulateQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponentIterator* bodiesIt);
	void AddPhysicsBodyToQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body);

	void BroadPhaseStep(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponentIterator* bodiesIt, std::vector<NarrowPhasePair>& _outNarrowPhase);
	void CalculateBroadphaseForBody(const types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body, std::vector<NarrowPhasePair>& _outNarrowPhase);

	void NarrowPhaseIntersectionSolutionStep(const std::vector<NarrowPhasePair>& narrowPhase);
	void ResolveNarrowPhase(const NarrowPhasePair& narrowPhase, math::GJK::Intersection& foundIntersection);
	void ResolveIntersection(const NarrowPhasePair& narrowPhase, const math::GJK::Intersection& intersection);

	static void GeneratePolygon(const PhysicsComponent& body, Polygon& _outPolygon);
};

}
}