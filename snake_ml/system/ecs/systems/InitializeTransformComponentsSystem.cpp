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

			body.m_shape.m_momentOfInertia = body.m_shape.m_mass * 
				(body.m_shape.m_dimensions.x * body.m_shape.m_dimensions.x + body.m_shape.m_dimensions.y * body.m_shape.m_dimensions.y) / 12.f;
			body.m_position = transform.m_position;
			body.m_acceleration = math::vector::zero;
			body.m_angularAcceleration = math::vector::zero;
			body.m_angularVelocity = math::vector::zero;
			body.m_rotation = transform.m_rotation;
			body.m_velocity = math::vector::zero;
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

}
}