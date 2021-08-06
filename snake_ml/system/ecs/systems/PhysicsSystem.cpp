// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "PhysicsSystem.h"

#include "system/drivers/OSDriver.h"

#include "system/ecs/components/MeshComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include "utils/math/algorithms/CollisionDetectionSAT.h"

#include <utility>

namespace snakeml
{
namespace system
{

TransformComponent* PhysicsSystem::s_emptyTransformComponent = new TransformComponent();
MeshComponent* PhysicsSystem::s_emptyMeshComponent = new MeshComponent();

PhysicsSystem::PhysicsSystem()
{
	uint32_t levelWidth = -1, levelHeight = -1;
	IOSDriver::GetInstance()->GetAppDimensions(levelWidth, levelHeight);
	const float quadTreeHalfWidth = static_cast<float>(levelWidth / 2 + k_quadTreeHalfDimensionsMargin), quadTreeHalfHeight = static_cast<float>(levelHeight / 2 + k_quadTreeHalfDimensionsMargin);

	m_quadTreeHalfDimensions = { quadTreeHalfWidth, quadTreeHalfHeight, 0.f };
}

void PhysicsSystem::Update(double deltaTime)
{
	//deltaTime = k_physicsTimeStep; // debug
	float timeToSimulate = deltaTime;

	do
	{
		const float dt = timeToSimulate > k_physicsTimeStep ? k_physicsTimeStep : timeToSimulate;
		timeToSimulate -= dt;

		types::QuadTree<PhysicsComponent> qt(types::QuadTree<PhysicsComponent>::Rectangle{ math::vector::zero, m_quadTreeHalfDimensions });
		std::vector<NarrowPhasePair> narrowPhase;

		PhysicsComponentIterator* bodiesIt = ECSManager::GetInstance()->GetComponentsPool().GetComponents<PhysicsComponentIterator>();

		SimulatePhysics(bodiesIt, dt);
		UpdateAABBs(bodiesIt);
		PopulateQuadTree(qt, bodiesIt);
		BroadPhaseStep(qt, bodiesIt, narrowPhase);
		NarrowPhaseIntersectionSolutionStep(narrowPhase);
	}
	while (!math::IsNearlyZero(timeToSimulate, math::k_default_epsilon));
}

void PhysicsSystem::SimulatePhysics(const PhysicsComponentIterator* bodiesIt, double dt)
{
	for (size_t idx = 0; idx < bodiesIt->Size(); ++idx)
	{
		PhysicsComponent& body = bodiesIt->At(idx);
		TransformComponent& transform = GetTransformComponent(body);

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

void PhysicsSystem::UpdateAABBs(const PhysicsComponentIterator* bodiesIt)
{
	for (size_t i = 0u; i < bodiesIt->Size(); ++i)
	{
		PhysicsComponent& body = bodiesIt->At(i);
		if (body.m_isDynamic)
		{
			UpdateAABB(body);
		}
	}
}

void PhysicsSystem::UpdateAABB(PhysicsComponent& body)
{
	// Generate Bounding Box
	BoundingBox boundingBox;
	GenerateBoundingBox(body.m_shape.m_dimensions, boundingBox);

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

void PhysicsSystem::PopulateQuadTree(types::QuadTree<PhysicsComponent>& quadTree, const PhysicsComponentIterator* bodiesIt)
{
	for (size_t i = 0u; i < bodiesIt->Size(); ++i)
	{
		PhysicsComponent& body = bodiesIt->At(i);
		AddPhysicsBodyToQuadTree(quadTree, body);
	}
}

void PhysicsSystem::AddPhysicsBodyToQuadTree(types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body)
{
	quadTree.AddObject(types::QuadTree<PhysicsComponent>::Object{ {body.m_position, {(body.m_aabb.max - body.m_aabb.min) / 2.f}}, body });
}

void PhysicsSystem::BroadPhaseStep(types::QuadTree<PhysicsComponent>& quadTree, const PhysicsComponentIterator* bodiesIt, std::vector<NarrowPhasePair>& _outNarrowPhase)
{
	for (size_t i = 0u; i < bodiesIt->Size(); ++i)
	{
		PhysicsComponent& body = bodiesIt->At(i);
		if (!body.m_isDynamic)
		{
			continue;
		}

		CalculateBroadphaseForBody(quadTree, body, _outNarrowPhase);
	}
}

void PhysicsSystem::CalculateBroadphaseForBody(const types::QuadTree<PhysicsComponent>& quadTree, PhysicsComponent& body, std::vector<NarrowPhasePair>& _outNarrowPhase)
{
	const types::QuadTree<PhysicsComponent>::Rectangle boundary = { body.m_position, body.m_aabb.max - body.m_aabb.min };
	std::vector<const types::QuadTree<PhysicsComponent>::Object const*> objects;

	quadTree.GetObjects(boundary, objects);

	for (auto object : objects)
	{
		if (&object->userData.m_entityId == &body.m_entityId)
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

		IOSDriver::GetInstance()->LogMessage(L"INTERSECTION\n");
	}
}

void PhysicsSystem::GenerateBoundingBox(const math::vector& shapeDimensions, BoundingBox& _outBBox)
{
	_outBBox = 
	{
		math::vector{-shapeDimensions.x / 2.f, -shapeDimensions.y / 2.f, 0.f},
		math::vector{-shapeDimensions.x / 2.f, +shapeDimensions.y / 2.f, 0.f},
		math::vector{+shapeDimensions.x / 2.f, -shapeDimensions.y / 2.f, 0.f},
		math::vector{+shapeDimensions.x / 2.f, +shapeDimensions.y / 2.f, 0.f}
	};
}

void PhysicsSystem::GeneratePolygon(const PhysicsComponent& body, Polygon& _outPolygon)
{
	const MeshComponent& mesh = GetMeshComponent(body);
	const TransformComponent& transform = GetTransformComponent(body);

	const math::matrix scaleMatrix = math::ScaleMatrix(transform.m_scale.x, transform.m_scale.y, transform.m_scale.z);
	const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(transform.m_rotation.y), math::ConvertToRadians(transform.m_rotation.x), math::ConvertToRadians(transform.m_rotation.z));
	const math::matrix translationMatrix = math::TranslationMatrix(transform.m_position.x, transform.m_position.y, transform.m_position.z);
	const math::matrix modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	for (const auto& vertex : mesh.m_vertices)
	{
		const math::vector v = { vertex.first.m_x, vertex.first.m_y, vertex.first.m_z };
		_outPolygon.push_back(modelMatrix * v);
	}
}

TransformComponent& PhysicsSystem::GetTransformComponent(const PhysicsComponent& body)
{
	const Entity* entityPtr = ECSManager::GetInstance()->GetEntity(body.m_entityId);
	ASSERT(entityPtr, "[PhysicsSystem::GeneratePolygon] : Missconfigured entity.");
	if (entityPtr)
	{
		TransformComponent* transformPtr = entityPtr->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>();
		ASSERT(transformPtr, "[PhysicsSystem::GeneratePolygon] : Missconfigured entity.");
		if (transformPtr)
		{
			return *transformPtr;
		}
	}
	return *s_emptyTransformComponent;
}

MeshComponent& PhysicsSystem::GetMeshComponent(const PhysicsComponent& body)
{
	const Entity* entityPtr = ECSManager::GetInstance()->GetEntity(body.m_entityId);
	ASSERT(entityPtr, "[PhysicsSystem::GeneratePolygon] : Missconfigured entity.");
	if (entityPtr)
	{
		MeshComponent* meshPtr = entityPtr->m_components.at(ComponentType::MeshComponent)->As<MeshComponent>();
		ASSERT(meshPtr, "[PhysicsSystem::GeneratePolygon] : Missconfigured entity.");
		if (meshPtr)
		{
			return *meshPtr;
		}
	}
	return *s_emptyMeshComponent;
}

}
}