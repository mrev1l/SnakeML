// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "PhysicsSystem.h"

#include "system/ecs/components/MeshComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include "utils/math/algorithms/CollisionDetectionSAT.h"

#include <utility>

// TODO : REPLACE WITH LOGGING
#include "system/drivers/win/os/WinDriver.h"
namespace snakeml
{
namespace system
{

void PhysicsSystem::Update(double deltaTime)
{
	static constexpr float k_physicsTimeStep = 1.f / 144.f; // 1 simulation step per frame per 144 fps
	//deltaTime = k_physicsTimeStep; // debug
	float timeToSimulate = deltaTime;

	do
	{
		const float dt = timeToSimulate > k_physicsTimeStep ? k_physicsTimeStep : timeToSimulate;
		timeToSimulate -= dt;

		types::QuadTree<PhysicsComponent> qt(types::QuadTree<PhysicsComponent>::Rectangle{ {0.f, 0.f, 0.f}, {370.f, 370.f, 0.f} });
		std::vector<NarrowPhasePair> narrowPhase;

		PhysicsComponentIterator* bodies = (PhysicsComponentIterator*)ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::PhysicsComponent);
		PhysicsComponent* bodiesArr = (PhysicsComponent*)bodies->GetData();

		SimulatePhysics(bodiesArr, bodies->Size(), dt);
		UpdateAABBs(bodiesArr, bodies->Size());
		PopulateQuadTree(qt, bodiesArr, bodies->Size());
		BroadPhaseStep(qt, bodiesArr, bodies->Size(), narrowPhase);
		NarrowPhaseIntersectionSolutionStep(narrowPhase);
	}
	while (!math::IsNearlyZero(timeToSimulate, math::k_default_epsilon));
}

void PhysicsSystem::SimulatePhysics(PhysicsComponent* physicsBodies, size_t bodiesCount, double dt)
{
	for (size_t idx = 0; idx < bodiesCount; ++idx)
	{
		PhysicsComponent& body = physicsBodies[idx];
		Entity* entityPtr = ECSManager::GetInstance()->GetEntity(body.m_entityId);
		ASSERT(entityPtr, "(PhysicsSystem::Update) : Invalid entity ptr.");

		TransformComponent& transform = *(TransformComponent*)entityPtr->m_components[ComponentType::TransformComponent];

		if (body.m_isDynamic)
		{
			SimulatePhysicsStep(body, transform, dt);
		}
	}
}

void PhysicsSystem::SimulatePhysicsStep(PhysicsComponent& body, TransformComponent& transform, double dt)
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

void PhysicsSystem::UpdateAABBs(PhysicsComponent* physicsBodies, size_t bodiesCount)
{
	for (size_t i = 0u; i < bodiesCount; ++i)
	{
		PhysicsComponent& body = physicsBodies[i];
		if (body.m_isDynamic)
		{
			UpdateAABB(body);
		}
	}
}

void PhysicsSystem::UpdateAABB(PhysicsComponent& body)
{
	// Create Bounding Box
	std::array<math::vector, 4> boundingBox =
	{
		math::vector{-body.m_shape.m_dimensions.x / 2.f, -body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{-body.m_shape.m_dimensions.x / 2.f, +body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{+body.m_shape.m_dimensions.x / 2.f, -body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{+body.m_shape.m_dimensions.x / 2.f, +body.m_shape.m_dimensions.y / 2.f, 0.f}
	};

	// Calc transform matrix for Bounding Box vertices
	const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(body.m_rotation.y), math::ConvertToRadians(body.m_rotation.x), math::ConvertToRadians(body.m_rotation.z));
	const math::matrix translationMatrix = math::TranslationMatrix(body.m_position.x, body.m_position.y, body.m_position.z);
	const math::matrix transformMatrix = rotationMatrix * translationMatrix;

	// Transform Bounding Box vertices
	for (auto& vertex : boundingBox)
	{
		vertex = transformMatrix * vertex;
	}

	// Construct AABB
	body.m_aabb = { {FLT_MAX, FLT_MAX, 0.f}, {-FLT_MAX, -FLT_MAX, 0.f} };
	for (auto vertex : boundingBox)
	{
		body.m_aabb.min.x = std::min(body.m_aabb.min.x, vertex.x);
		body.m_aabb.min.y = std::min(body.m_aabb.min.y, vertex.y);
		body.m_aabb.max.x = std::max(body.m_aabb.max.x, vertex.x);
		body.m_aabb.max.y = std::max(body.m_aabb.max.y, vertex.y);
	}
}

void PhysicsSystem::PopulateQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent* physicsBodies, size_t bodiesCount)
{
	for (size_t i = 0u; i < bodiesCount; ++i)
	{
		PhysicsComponent& body = physicsBodies[i];
		AddPhysicsBodyToQuadTree(quadTree, body);
	}
}

void PhysicsSystem::AddPhysicsBodyToQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body)
{
	quadTree.AddObject(types::QuadTree<PhysicsComponent>::Object{ {body.m_position, {(body.m_aabb.max - body.m_aabb.min) / 2.f}}, body });
}

void PhysicsSystem::BroadPhaseStep(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent* physicsBodies, size_t bodiesCount, std::vector<NarrowPhasePair>& _outNarrowPhase)
{
	for (size_t i = 0u; i < bodiesCount; ++i)
	{
		PhysicsComponent& body = physicsBodies[i];
		if (!body.m_isDynamic)
		{
			continue;
		}

		CalculateBroadphaseForBody(quadTree, body, _outNarrowPhase);
	}
}

void PhysicsSystem::CalculateBroadphaseForBody(const types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body, std::vector<NarrowPhasePair>& _outNarrowPhase)
{
	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	const types::QuadTree<PhysicsComponent>::Rectangle boundary = { body.m_position, body.m_aabb.max - body.m_aabb.min };
	std::vector<const types::QuadTree<PhysicsComponent>::Object const*> objects;

	quadTree.GetObjects(boundary, objects);

	for (auto object : objects)
	{
		if (&object->userData == &body)
		{
			continue;
		}
		const types::AABB& a = body.m_aabb;
		const types::AABB& b = object->userData.m_aabb;

		const bool areIntersecting = types::TestIntersection_AABB_AABB(a, b);

		if (areIntersecting)
		{
			Polygon polygonA, polygonB;
			
			GeneratePolygon(body, polygonA);
			GeneratePolygon(object->userData, polygonB);
			
			NarrowPhaseBody narrowPhaseBodyA(body, std::forward<Polygon>(polygonA));
			NarrowPhaseBody narrowPhaseBodyB(object->userData, std::forward<Polygon>(polygonB));

			_outNarrowPhase.emplace_back(std::move(NarrowPhasePair(std::forward<NarrowPhaseBody>(narrowPhaseBodyA), std::forward<NarrowPhaseBody>(narrowPhaseBodyB))));
		}
	}
}

void PhysicsSystem::NarrowPhaseIntersectionSolutionStep(const std::vector<NarrowPhasePair>& narrowPhase)
{
	math::GJK::Intersection foundIntersection;
	for (const auto& pair : narrowPhase)
	{
		ResolveNarrowPhase(pair, foundIntersection);

		if (foundIntersection.areIntersecting)
		{
			ResolveIntersection(pair, foundIntersection);
		}
	}
}

void PhysicsSystem::ResolveNarrowPhase(const NarrowPhasePair& narrowPhase, math::GJK::Intersection& _outFoundGJK_Intersection)
{
	const bool isSAT_IntersectionFound = math::SAT::TestIntersection(narrowPhase.a.polygon, narrowPhase.b.polygon);
	if (isSAT_IntersectionFound)
	{
		_outFoundGJK_Intersection = math::GJK::TestIntersection(narrowPhase.a.polygon, narrowPhase.b.polygon, narrowPhase.a.physicsObject.m_position, narrowPhase.b.physicsObject.m_position);
	}
}

void PhysicsSystem::ResolveIntersection(const NarrowPhasePair& narrowPhase, const math::GJK::Intersection& intersection)
{
	if (narrowPhase.a.physicsObject.m_isDynamic)
	{
		narrowPhase.a.physicsObject.m_position -= intersection.penetrationVector * intersection.penetrationDepth;

		math::vector reflectedVelocity = narrowPhase.a.physicsObject.m_velocity - (-intersection.penetrationVector * 2.f * narrowPhase.a.physicsObject.m_velocity.dot(-intersection.penetrationVector));
		narrowPhase.a.physicsObject.m_velocity = reflectedVelocity;
		narrowPhase.a.physicsObject.m_acceleration = math::vector::zero;

		OutputDebugString(L"INTERSECTION\n");
	}
}

void PhysicsSystem::GeneratePolygon(const PhysicsComponent& body, Polygon& _outPolygon)
{
	const Entity* entityPtr = ECSManager::GetInstance()->GetEntity(body.m_entityId);
	ASSERT(entityPtr, "[PhysicsSystem::GeneratePolygon] : Missconfigured entity.");
	const Entity& entity = *entityPtr;

	const MeshComponent& mesh = *(MeshComponent*)entity.m_components.at(ComponentType::MeshComponent);
	const TransformComponent& transform = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);

	const math::matrix scaleMatrix = math::ScaleMatrix(transform.m_scale.x, transform.m_scale.y, transform.m_scale.z);
	const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(transform.m_rotation.y), math::ConvertToRadians(transform.m_rotation.x), math::ConvertToRadians(transform.m_rotation.z));
	const math::matrix translationMatrix = math::TranslationMatrix(transform.m_position.x, transform.m_position.y, transform.m_position.z);
	const math::matrix modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	for (const auto& vertex : mesh.m_vertices)
	{
		math::vector v = { vertex.first.m_x, vertex.first.m_y, vertex.first.m_z };
		_outPolygon.push_back(modelMatrix * v);
	}
}

}
}