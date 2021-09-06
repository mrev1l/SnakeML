// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ConsumablesSystem.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/ConsumableComponent.h"
#include "system/ecs/components/PhysicsComponent.h"

namespace snakeml
{

ConsumablesSystem::ConsumablesSystem()
{
	if (PhysicsSystem* physicsSystem = ECSManager::GetInstance()->GetSystem<PhysicsSystem>())
	{
		physicsSystem->m_onCollisionEvent.Subscribe(this, std::bind(&ConsumablesSystem::OnCollisionEvent, this, std::placeholders::_1));
	}
	
}
void ConsumablesSystem::OnCollisionEvent(Collision collision)
{
	if (!VerifyCollision(collision))
	{
		return;
	}

	// need to hide the consumable
	// as a first-pass - move it out of sight
	Entity& consumable = ECSManager::GetInstance()->GetEntity(collision.entityB);
	PhysicsComponent& physicsComponent = *consumable.m_components[ComponentType::PhysicsComponent]->As<PhysicsComponent>();
	physicsComponent.m_position = vector::up * 1000.f;
}

bool ConsumablesSystem::VerifyCollision(Collision collision)
{
	const bool isExpectedPlayerEntityInPlace = DoesEntityContainComponent(collision.entityA, ComponentType::EntityControllerComponent);
	const bool isExpectedConsumableEntityInPlace = DoesEntityContainComponent(collision.entityB, ComponentType::ConsumableComponent);
	return isExpectedPlayerEntityInPlace && isExpectedConsumableEntityInPlace;
}

bool ConsumablesSystem::DoesEntityContainComponent(uint32_t entityId, ComponentType componentType)
{
	const Entity& entity = ECSManager::GetInstance()->GetEntity(entityId);
	return entity.m_components.contains(componentType);
}

}