// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeTransformComponentsSystem.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{
namespace system
{

void InitializeTransformComponentsSystem::Execute()
{
	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	for (const Entity& entity : entities)
	{
		if (entity.m_components.find(ComponentType::TransformComponent) != entity.m_components.end() &&
			entity.m_components.find(ComponentType::PhysicsComponent) != entity.m_components.end())
		{
			TransformComponent& transform = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);
			PhysicsComponent& body = *(PhysicsComponent*)entity.m_components.at(ComponentType::PhysicsComponent);

			InitPhysicsBody(transform, body);
		}
	}
	return; // TODO
	TransformComponentIterator* it = (TransformComponentIterator*)IComponent::CreateIterator(ComponentType::TransformComponent, 1);
	TransformComponent* transform = (TransformComponent*)it->At(0);
	transform->m_entityId = 0;
	transform->m_position = { 0.f, 0.f, 0.f };
	transform->m_rotation = { 0.f, 0.f, 0.f };
	transform->m_scale = { 1.f, 1.f, 1.f };

	PhysicsComponentIterator* phIt = (PhysicsComponentIterator*)IComponent::CreateIterator(ComponentType::PhysicsComponent, 1);
	PhysicsComponent& body = *(PhysicsComponent*)phIt->At(0);
	body.m_entityId = 0;
	body.m_shape = {{72.f, 72.f, 0.f}, 1.f, 1.f * (72.f * 72.f + 72.f * 72.f) / 12.f};
	body.m_position = transform->m_position;
	body.m_acceleration = math::vector::zero;
	body.m_angularAcceleration = math::vector::zero;
	body.m_angularVelocity = math::vector::zero;
	body.m_rotation = transform->m_rotation;
	body.m_velocity = math::vector::zero;

	ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::TransformComponent, it);
	ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::PhysicsComponent, phIt);
}

void InitializeTransformComponentsSystem::InitPhysicsBody(const TransformComponent& transform, PhysicsComponent& _outBody)
{
	_outBody.m_shape.m_momentOfInertia = _outBody.m_shape.m_mass *
		(_outBody.m_shape.m_dimensions.x * _outBody.m_shape.m_dimensions.x + _outBody.m_shape.m_dimensions.y * _outBody.m_shape.m_dimensions.y) / 12.f;
	_outBody.m_position = transform.m_position;
	_outBody.m_acceleration = math::vector::zero;
	_outBody.m_angularAcceleration = math::vector::zero;
	_outBody.m_angularVelocity = math::vector::zero;
	_outBody.m_rotation = transform.m_rotation;
	_outBody.m_velocity = math::vector::zero;

	InitAABB(_outBody);
}

void InitializeTransformComponentsSystem::InitAABB(PhysicsComponent& _outBody)
{
	// Create Bounding Box
	std::array<math::vector, 4> boundingBox =
	{
		math::vector{-_outBody.m_shape.m_dimensions.x / 2.f, -_outBody.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{-_outBody.m_shape.m_dimensions.x / 2.f, +_outBody.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{+_outBody.m_shape.m_dimensions.x / 2.f, -_outBody.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{+_outBody.m_shape.m_dimensions.x / 2.f, +_outBody.m_shape.m_dimensions.y / 2.f, 0.f}
	};

	// Calc transform matrix for Bounding Box vertices
	const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(_outBody.m_rotation.y), math::ConvertToRadians(_outBody.m_rotation.x), math::ConvertToRadians(_outBody.m_rotation.z));
	const math::matrix translationMatrix = math::TranslationMatrix(_outBody.m_position.x, _outBody.m_position.y, _outBody.m_position.z);
	const math::matrix transformMatrix = rotationMatrix * translationMatrix;

	// Transform Bounding Box vertices
	for (auto& vertex : boundingBox)
	{
		vertex = transformMatrix * vertex;
	}

	// Construct AABB
	_outBody.m_aabb = { {FLT_MAX, FLT_MAX, 0.f}, {-FLT_MAX, -FLT_MAX, 0.f} };
	for (auto vertex : boundingBox)
	{
		_outBody.m_aabb.min.x = std::min(_outBody.m_aabb.min.x, vertex.x);
		_outBody.m_aabb.min.y = std::min(_outBody.m_aabb.min.y, vertex.y);
		_outBody.m_aabb.max.x = std::max(_outBody.m_aabb.max.x, vertex.x);
		_outBody.m_aabb.max.y = std::max(_outBody.m_aabb.max.y, vertex.y);
	}
}

}
}