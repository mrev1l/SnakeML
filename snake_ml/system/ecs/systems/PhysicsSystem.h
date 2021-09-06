#pragma once

#include "system/ecs/Entity.h"
#include "system/ecs/ISystem.h"
#include "system/ecs/components/PhysicsComponent.h"

#include "utils/math/algorithms/CollisionDetectionGJK.h"
#include "utils/patterns/event.h"
#include "utils/types/QuadTree.h"

namespace snakeml
{

class TransformComponent;
class MeshComponent;

struct Collision
{
	uint32_t			entityA;
	uint32_t			entityB;
	CollisionChannel	collisionChannel;
};

class PhysicsSystem : public ISystemCastableImpl<PhysicsSystem>
{
public:
	PhysicsSystem();

	void Update(float deltaTime) override;

	Event<PhysicsSystem, Collision> m_onCollisionEvent;

private:
	using BoundingBox = std::array<vector, 4>;
	using Polygon = std::vector<vector>;
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

	void SimulatePhysics(const PhysicsComponentIterator& bodiesIt, float dt);
	void SimulatePhysicsStep(PhysicsComponent& body, TransformComponent& transform, float dt);

	void UpdateAABBs(const PhysicsComponentIterator& bodiesIt);

	void PopulateQuadTree(QuadTree<PhysicsComponent>& quadTree, const PhysicsComponentIterator& bodiesIt);
	void AddPhysicsBodyToQuadTree(QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body);

	void BroadPhaseStep(QuadTree<PhysicsComponent>& quadTree, const PhysicsComponentIterator& bodiesIt, std::vector<NarrowPhasePair>& _outNarrowPhase);
	void CalculateBroadphaseForBody(const QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body, std::vector<NarrowPhasePair>& _outNarrowPhase);

	void NarrowPhaseIntersectionSolutionStep(const std::vector<NarrowPhasePair>& narrowPhase);
	void ResolveNarrowPhase(const NarrowPhasePair& narrowPhase, GJK::Intersection& foundIntersection);
	void ResolveIntersection(const NarrowPhasePair& narrowPhase, const GJK::Intersection& intersection);

	static TransformComponent& GetTransformComponent(const PhysicsComponent& body);
	static MeshComponent& GetMeshComponent(const PhysicsComponent& body);
	static void GeneratePolygon(const PhysicsComponent& body, Polygon& _outPolygon);
	static bool IsNarrowPhasePairPresent(std::vector<NarrowPhasePair>& narrowPhase, const PhysicsComponent& bodyA, const PhysicsComponent& bodyB);

	vector m_quadTreeHalfDimensions = vector::zero;

	static TransformComponent*	s_emptyTransformComponent;
	static MeshComponent*		s_emptyMeshComponent;
	static constexpr float		k_physicsTimeStep = 1.f / 144.f;		// 1 simulation step per frame per 144 fps
	static constexpr uint32_t	k_quadTreeHalfDimensionsMargin = 10u;
};

}
