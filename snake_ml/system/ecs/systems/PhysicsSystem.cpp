// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "PhysicsSystem.h"

#include "system/drivers/OSDriver.h"

#include "system/ecs/components/MeshComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include "utils/math/algorithms/CollisionDetectionSAT.h"

#include <utility>

namespace snakeml
{

TransformComponent* PhysicsSystem::s_emptyTransformComponent = new TransformComponent();
MeshComponent* PhysicsSystem::s_emptyMeshComponent = new MeshComponent();

PhysicsSystem::PhysicsSystem() : ISystemCastableImpl<PhysicsSystem>()
{
	uint32_t levelWidth = -1, levelHeight = -1;
	IOSDriver::GetInstance()->GetAppDimensions(levelWidth, levelHeight);
	const float quadTreeHalfWidth = static_cast<float>(levelWidth / 2 + k_quadTreeHalfDimensionsMargin), quadTreeHalfHeight = static_cast<float>(levelHeight / 2 + k_quadTreeHalfDimensionsMargin);

	m_quadTreeHalfDimensions = { quadTreeHalfWidth, quadTreeHalfHeight, 0.f };
}

void PhysicsSystem::Update(float deltaTime)
{
	//deltaTime = k_physicsTimeStep; // debug
	float timeToSimulate = deltaTime;

	do
	{
		const float dt = timeToSimulate > k_physicsTimeStep ? k_physicsTimeStep : timeToSimulate;
		timeToSimulate -= dt;

		QuadTree<PhysicsComponent> qt(QuadTree<PhysicsComponent>::Rectangle{ vector::zero, m_quadTreeHalfDimensions });
		std::vector<NarrowPhasePair> narrowPhase;

		PhysicsComponentIterator* bodiesIt = ECSManager::GetInstance()->GetComponents<PhysicsComponentIterator>();

		SimulatePhysics(*bodiesIt, dt);
		UpdateAABBs(*bodiesIt);
		PopulateQuadTree(qt, *bodiesIt);
		BroadPhaseStep(qt, *bodiesIt, narrowPhase);
		NarrowPhaseIntersectionSolutionStep(narrowPhase);
	}
	while (!IsNearlyZero(timeToSimulate, k_default_epsilon));
}

void PhysicsSystem::SimulatePhysics(const PhysicsComponentIterator& bodiesIt, float dt)
{
	for (PhysicsComponent& body : bodiesIt)
	{
		TransformComponent& transform = GetTransformComponent(body);

		// TODO : Deprecate ? if (body.m_isDynamic)
		if(body.m_collisionChannel != CollisionChannel::Static)
		{
			SimulatePhysicsStep(body, transform, dt);
		}
	}
}

void PhysicsSystem::SimulatePhysicsStep(PhysicsComponent& body, TransformComponent& transform, float dt)
{
	// Linear movement integration
	body.m_velocity += body.m_acceleration * dt;
	body.m_position += body.m_velocity * dt;

	// Angular movement integration
	body.m_angularVelocity += body.m_angularAcceleration * dt;
	body.m_rotation += body.m_angularVelocity * dt;

	// Update transform
	transform.m_position = body.m_position;
	transform.m_rotation = body.m_rotation;
}

void PhysicsSystem::UpdateAABBs(const PhysicsComponentIterator& bodiesIt)
{
	for (PhysicsComponent& body : bodiesIt)
	{
		// TODO : Deprecate ? if (body.m_isDynamic)
		if (body.m_collisionChannel != CollisionChannel::Static)
		{
			body.m_aabb = AABB::GenerateAABB(body.m_shape.m_dimensions, body.m_position, body.m_rotation);
		}
	}
}

void PhysicsSystem::PopulateQuadTree(QuadTree<PhysicsComponent>& quadTree, const PhysicsComponentIterator& bodiesIt)
{
	for (PhysicsComponent& body : bodiesIt)
	{
		AddPhysicsBodyToQuadTree(quadTree, body);
	}
}

void PhysicsSystem::AddPhysicsBodyToQuadTree(QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body)
{
	quadTree.AddObject(QuadTree<PhysicsComponent>::Object{ {body.m_position, {(body.m_aabb.max - body.m_aabb.min) / 2.f}}, body });
}

void PhysicsSystem::BroadPhaseStep(QuadTree<PhysicsComponent>& quadTree, const PhysicsComponentIterator& bodiesIt, std::vector<NarrowPhasePair>& _outNarrowPhase)
{
	for (PhysicsComponent& body : bodiesIt)
	{
		// TODO : Deprecate ? if (!body.m_isDynamic)
		if (body.m_collisionChannel == CollisionChannel::Static)
		{
			continue;
		}

		CalculateBroadphaseForBody(quadTree, body, _outNarrowPhase);
	}
}

void PhysicsSystem::CalculateBroadphaseForBody(const QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body, std::vector<NarrowPhasePair>& _outNarrowPhase)
{
	const QuadTree<PhysicsComponent>::Rectangle boundary = { body.m_position, body.m_aabb.max - body.m_aabb.min };
	std::vector<const QuadTree<PhysicsComponent>::Object*> objects;

	quadTree.GetObjects(boundary, objects);

	for (auto object : objects)
	{
		PhysicsComponent& a = body;
		PhysicsComponent& b = object->userData;
		
		const bool isTheSameEntity = a.m_entityId == b.m_entityId;
		const bool isCollisionFilteredOut = (a.m_collisionFilter & b.m_collisionChannel) == CollisionChannel::None;
		const bool isAlreadyDetected = IsNarrowPhasePairPresent(_outNarrowPhase, a, b);

		if(isTheSameEntity || isCollisionFilteredOut || isAlreadyDetected)
		{
			continue;
		}

		const bool areIntersecting = AABB::TestIntersection_AABB_AABB(a.m_aabb, b.m_aabb);

		if (areIntersecting)
		{
			Polygon polygonA, polygonB;
			
			GeneratePolygon(a, polygonA);
			GeneratePolygon(b, polygonB);
			
			NarrowPhaseBody narrowPhaseBodyA(a, std::forward<Polygon>(polygonA));
			NarrowPhaseBody narrowPhaseBodyB(b, std::forward<Polygon>(polygonB));

			_outNarrowPhase.emplace_back(std::move(NarrowPhasePair(std::forward<NarrowPhaseBody>(narrowPhaseBodyA), std::forward<NarrowPhaseBody>(narrowPhaseBodyB))));
		}
	}
}

void PhysicsSystem::NarrowPhaseIntersectionSolutionStep(const std::vector<NarrowPhasePair>& narrowPhase)
{
	GJK::Intersection foundIntersection;
	for (const auto& pair : narrowPhase)
	{
		ResolveNarrowPhase(pair, foundIntersection);

		if (foundIntersection.areIntersecting)
		{
			Collision collisionData =
			{
				pair.a.physicsObject.m_entityId,
				pair.b.physicsObject.m_entityId,
				pair.b.physicsObject.m_collisionChannel
			};
			m_onCollisionEvent.Dispatch(collisionData);
			// this is first-pass
			//ResolveIntersection(pair, foundIntersection)
			IOSDriver::GetInstance()->LogMessage(L"INTERSECTION\n");
		}
	}
}

void PhysicsSystem::ResolveNarrowPhase(const NarrowPhasePair& narrowPhase, GJK::Intersection& _outFoundGJK_Intersection)
{
	const bool isSAT_IntersectionFound = SAT::TestIntersection(narrowPhase.a.polygon, narrowPhase.b.polygon);
	if (isSAT_IntersectionFound)
	{
		_outFoundGJK_Intersection = GJK::TestIntersection(narrowPhase.a.polygon, narrowPhase.b.polygon, narrowPhase.a.physicsObject.m_position, narrowPhase.b.physicsObject.m_position);
	}
}

void PhysicsSystem::ResolveIntersection(const NarrowPhasePair& narrowPhase, const GJK::Intersection& intersection)
{
	// TODO : Deprecate ? if (narrowPhase.a.physicsObject.m_isDynamic)
	if (narrowPhase.a.physicsObject.m_collisionChannel == CollisionChannel::Dynamic)
	{
		narrowPhase.a.physicsObject.m_position -= intersection.penetrationVector * intersection.penetrationDepth;

		vector reflectedVelocity = narrowPhase.a.physicsObject.m_velocity - (-intersection.penetrationVector * 2.f * narrowPhase.a.physicsObject.m_velocity.dot(-intersection.penetrationVector));
		narrowPhase.a.physicsObject.m_velocity = reflectedVelocity;
		narrowPhase.a.physicsObject.m_acceleration = vector::zero;
	}
}

void PhysicsSystem::GeneratePolygon(const PhysicsComponent& body, Polygon& _outPolygon)
{
	const MeshComponent& mesh = GetMeshComponent(body);
	const TransformComponent& transform = GetTransformComponent(body);

	const matrix scaleMatrix = ScaleMatrix(transform.m_scale.x, transform.m_scale.y, transform.m_scale.z);
	const matrix rotationMatrix = RotationMatrix(ConvertToRadians(transform.m_rotation.y), ConvertToRadians(transform.m_rotation.x), ConvertToRadians(transform.m_rotation.z));
	const matrix translationMatrix = TranslationMatrix(transform.m_position.x, transform.m_position.y, transform.m_position.z);
	const matrix modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	for (const auto& vertex : mesh.m_vertices)
	{
		const vector v = { vertex.first.x, vertex.first.y, vertex.first.z };
		_outPolygon.push_back(modelMatrix * v);
	}
}

bool PhysicsSystem::IsNarrowPhasePairPresent(std::vector<NarrowPhasePair>& narrowPhase, const PhysicsComponent& bodyA, const PhysicsComponent& bodyB)
{
	return std::find_if(
		narrowPhase.begin(),
		narrowPhase.end(),
		[bodyA, bodyB](const NarrowPhasePair& pair) -> bool
		{
			const bool aBodyIsPresent = pair.a.physicsObject.m_entityId == bodyA.m_entityId || pair.b.physicsObject.m_entityId == bodyA.m_entityId;
			const bool bBodyIsPresent = pair.a.physicsObject.m_entityId == bodyB.m_entityId || pair.b.physicsObject.m_entityId == bodyB.m_entityId;
			return aBodyIsPresent && bBodyIsPresent;
		}
	) != narrowPhase.end();
}

TransformComponent& PhysicsSystem::GetTransformComponent(const PhysicsComponent& body)
{
	const Entity& entity = ECSManager::GetInstance()->GetEntity(body.m_entityId);
	TransformComponent* transformPtr = entity.m_components.at(ComponentType::TransformComponent)->As<TransformComponent>();
	ASSERT(transformPtr, "[PhysicsSystem::GeneratePolygon] : Missconfigured entity.");
	if (transformPtr)
	{
		return *transformPtr;
	}
	return *s_emptyTransformComponent;
}

MeshComponent& PhysicsSystem::GetMeshComponent(const PhysicsComponent& body)
{
	const Entity& entity = ECSManager::GetInstance()->GetEntity(body.m_entityId);
	MeshComponent* meshPtr = entity.m_components.at(ComponentType::MeshComponent)->As<MeshComponent>();
	ASSERT(meshPtr, "[PhysicsSystem::GeneratePolygon] : Missconfigured entity.");
	if (meshPtr)
	{
		return *meshPtr;
	}
	return *s_emptyMeshComponent;
}

}
