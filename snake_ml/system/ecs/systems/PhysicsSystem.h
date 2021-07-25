#pragma once

#include "system/ecs/ISystem.h"
#include "system/ecs/Entity.h"
#include "utils/types/QuadTree.h"

namespace snakeml
{
namespace system
{

class PhysicsComponent;
class TransformComponent;

struct Intersection
{
	bool areIntersecting = false;
	float penetrationDepth = 0.f;
	math::vector penetrationVector = math::vector::zero;
};

class PhysicsSystem : public ISystem
{
public:
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

	void SimulatePhysics(PhysicsComponent* physicsBodies, size_t bodiesCount, double dt);
	void SimulatePhysicsStep(PhysicsComponent& body, TransformComponent& transform, double dt);

	void UpdateAABBs(PhysicsComponent* physicsBodies, size_t bodiesCount);
	void UpdateAABB(PhysicsComponent& body);

	void PopulateQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent* physicsBodies, size_t bodiesCount);
	void AddPhysicsBodyToQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body);

	void BroadPhaseStep(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent* physicsBodies, size_t bodiesCount, std::vector<NarrowPhasePair>& _outNarrowPhase);
	void CalculateBroadphaseForBody(const types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body, std::vector<NarrowPhasePair>& _outNarrowPhase);

	void NarrowPhaseIntersectionSolutionStep(const std::vector<NarrowPhasePair>& narrowPhase);
	void ResolveNarrowPhase(const NarrowPhasePair& narrowPhase, Intersection& foundIntersection);
	void ResolveIntersection(const NarrowPhasePair& narrowPhase, const Intersection& intersection);

	static void GeneratePolygon(const PhysicsComponent& body, Polygon& _outPolygon);
};

}
}