// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeDebugRenderComponentsSystem.h"

#include "system/ecs/components/DebugRenderComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{

void InitializeDebugRenderComponentsSystem::Execute()
{
	if (m_targetEntities.empty()) // init for every entity
	{
		PhysicsComponentIterator* physicsComponentsIt = ECSManager::GetInstance()->GetComponents<PhysicsComponentIterator>();
		DebugRenderComponentIterator* debugRenderComponents = ECSManager::GetInstance()->GetComponents<DebugRenderComponentIterator>();

		for (size_t idx = 0; idx < physicsComponentsIt->Size(); ++idx)
		{
			DebugRenderComponent& debugRenderComponent = debugRenderComponents->Add();
			const PhysicsComponent& physicsComponent = physicsComponentsIt->At(idx);

			Entity& entity = ECSManager::GetInstance()->GetEntity(physicsComponent.m_entityId);

			InitDebugRenderComponent(entity, physicsComponent, debugRenderComponent);
		}
	}
	else
	{
		DebugRenderComponentIterator* debugRenderComponents = ECSManager::GetInstance()->GetComponents<DebugRenderComponentIterator>();
		for (int32_t entityId : m_targetEntities)
		{
			Entity& entity = ECSManager::GetInstance()->GetEntity(entityId);

			DebugRenderComponent& debugRenderComponent = debugRenderComponents->Add();
			const PhysicsComponent& physicsComponent = *entity.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();

			InitDebugRenderComponent(entity, physicsComponent, debugRenderComponent);
		}
	}
}

void InitializeDebugRenderComponentsSystem::InitDebugRenderComponent(Entity& entity, const PhysicsComponent& physicsComp, DebugRenderComponent& debugRenderComponent)
{
	debugRenderComponent.m_entityId = physicsComp.m_entityId;
	debugRenderComponent.m_debugAABB = physicsComp.m_aabb;

	entity.m_components.insert({ ComponentType::DebugRenderComponent, &debugRenderComponent });
}

}