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
class MeshComponent;

class PhysicsSystem : public ISystem
{
public:
	PhysicsSystem();

	void Update(double deltaTime) override;

private:
	using BoundingBox = std::array<math::vector, 4>;
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

	void SimulatePhysics(const PhysicsComponentIterator* bodiesIt, double dt);
	void SimulatePhysicsStep(PhysicsComponent& body, TransformComponent& transform, double dt);

	void UpdateAABBs(const PhysicsComponentIterator* bodiesIt);
	void UpdateAABB(PhysicsComponent& body);

	void PopulateQuadTree(types::QuadTree<PhysicsComponent>& quadTree, const PhysicsComponentIterator* bodiesIt);
	void AddPhysicsBodyToQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body);

	void BroadPhaseStep(types::QuadTree<PhysicsComponent>& quadTree, const PhysicsComponentIterator* bodiesIt, std::vector<NarrowPhasePair>& _outNarrowPhase);
	void CalculateBroadphaseForBody(const types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body, std::vector<NarrowPhasePair>& _outNarrowPhase);

	void NarrowPhaseIntersectionSolutionStep(const std::vector<NarrowPhasePair>& narrowPhase);
	void ResolveNarrowPhase(const NarrowPhasePair& narrowPhase, math::GJK::Intersection& foundIntersection);
	void ResolveIntersection(const NarrowPhasePair& narrowPhase, const math::GJK::Intersection& intersection);

	static TransformComponent& GetTransformComponent(const PhysicsComponent& body);
	static MeshComponent& GetMeshComponent(const PhysicsComponent& body);
	static void GenerateBoundingBox(const math::vector& shapeDimensions, BoundingBox& _outBBox);
	static void GeneratePolygon(const PhysicsComponent& body, Polygon& _outPolygon);

	math::vector m_quadTreeHalfDimensions = math::vector::zero;

	static TransformComponent*	s_emptyTransformComponent;
	static MeshComponent*		s_emptyMeshComponent;
	static constexpr float		k_physicsTimeStep = 1.f / 144.f;		// 1 simulation step per frame per 144 fps
	static constexpr uint32_t	k_quadTreeHalfDimensionsMargin = 10u;
};

}
}