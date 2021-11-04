// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializePhysicsComponentsSystem.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{

void InitializePhysicsComponentsSystem::Execute()
{
	if (m_targetEntities.empty()) // init all entities
	{
		const std::unordered_map<uint32_t, Entity>& entities = ECSManager::GetInstance()->GetEntities();
		for (const std::pair<uint32_t, Entity>& entity : entities)
		{
			if (entity.second.m_components.find(ComponentType::TransformComponent) != entity.second.m_components.end() && // TODO Contains!
				entity.second.m_components.find(ComponentType::PhysicsComponent) != entity.second.m_components.end())	  // TODO Contains!
			{
				TransformComponent& transform = *entity.second.m_components.at(ComponentType::TransformComponent)->As<TransformComponent>();
				PhysicsComponent& body = *entity.second.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();

				InitPhysicsBody(transform, body);
			}
		}
	}
	else
	{
		for (uint32_t entityId : m_targetEntities)
		{
			Entity& entity = ECSManager::GetInstance()->GetEntity(entityId);
			ASSERT(entity.m_components.contains(ComponentType::PhysicsComponent) && entity.m_components.contains(ComponentType::TransformComponent),
				"[InitializePhysicsComponentsSystem::Execute] : incorrect occurence in m_entitiesToInit.");

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
	_outBody.m_acceleration = vector::zero;
	_outBody.m_angularAcceleration = vector::zero;
	_outBody.m_angularVelocity = vector::zero;
	_outBody.m_rotation = transform.m_rotation;
	_outBody.m_velocity = vector::zero;

	_outBody.m_aabb = AABB::GenerateAABB(_outBody.m_shape.m_dimensions, _outBody.m_position, _outBody.m_rotation);
}

}
