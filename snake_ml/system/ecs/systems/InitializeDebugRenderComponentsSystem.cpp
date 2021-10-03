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
	PhysicsComponentIterator* physicsComponentsIt = ECSManager::GetInstance()->GetComponents<PhysicsComponentIterator>();
	//DebugRenderComponentIterator* debugRenderComponents = IComponent::CreateIterator(ComponentType::DebugRenderComponent, physicsComponentsIt->Size())->As<DebugRenderComponentIterator>();
	DebugRenderComponentIterator* debugRenderComponents = ECSManager::GetInstance()->GetComponents<DebugRenderComponentIterator>();
	
	for (size_t idx = 0; idx < physicsComponentsIt->Size(); ++idx)
	{
		DebugRenderComponent& debugRenderComponent = debugRenderComponents->Add();
		PhysicsComponent& physicsComponent = physicsComponentsIt->At(idx);
		
		debugRenderComponent.m_entityId = physicsComponent.m_entityId;
		debugRenderComponent.m_debugAABB = physicsComponent.m_aabb;

		Entity& entity = ECSManager::GetInstance()->GetEntity(physicsComponent.m_entityId);
		entity.m_components.insert({ ComponentType::DebugRenderComponent, &debugRenderComponent });
	}

	//ECSManager::GetInstance()->InsertComponents<DebugRenderComponentIterator>(debugRenderComponents);
}

}