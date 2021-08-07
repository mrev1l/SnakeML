// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializePhysicsComponentsSystem.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{
namespace system
{

void InitializePhysicsComponentsSystem::Execute()
{
	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	for (const Entity& entity : entities)
	{
		if (entity.m_components.find(ComponentType::TransformComponent) != entity.m_components.end() &&
			entity.m_components.find(ComponentType::PhysicsComponent) != entity.m_components.end())
		{
			TransformComponent& transform = *entity.m_components.at(ComponentType::TransformComponent)->As<TransformComponent>();
			PhysicsComponent& body = *entity.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();

			InitPhysicsBody(transform, body);
		}
	}
}

void InitializePhysicsComponentsSystem::InitPhysicsBody(const TransformComponent& transform, PhysicsComponent& _outBody)
{
	_outBody.m_shape.m_momentOfInertia = _outBody.m_shape.m_mass *
		(_outBody.m_shape.m_dimensions.x * _outBody.m_shape.m_dimensions.x + _outBody.m_shape.m_dimensions.y * _outBody.m_shape.m_dimensions.y) / 12.f;
	_outBody.m_position = transform.m_position;
	_outBody.m_acceleration = math::vector::zero;
	_outBody.m_angularAcceleration = math::vector::zero;
	_outBody.m_angularVelocity = math::vector::zero;
	_outBody.m_rotation = transform.m_rotation;
	_outBody.m_velocity = math::vector::zero;

	_outBody.m_aabb = types::AABB::GenerateAABB(_outBody.m_shape.m_dimensions, _outBody.m_position, _outBody.m_rotation);
}

}
}