// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeEntitiesSystem.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

namespace snakeml
{
namespace system
{

void InitializeEntitiesSystem::Execute()
{
	const ComponentsPool& componentsPool = ECSManager::GetInstance()->GetComponentsPool();
	std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();

	for (size_t i = static_cast<size_t>(ComponentType::MaterialComponent);
		i < static_cast<size_t>(ComponentType::Size);
		++i)
	{
		Iterator* it = componentsPool.GetComponents(static_cast<ComponentType>(i));
		for (size_t componentIdx = 0; componentIdx < it->Num(); ++componentIdx)
		{
			IComponent* component = it->At(componentIdx);

			std::vector<Entity>::iterator entityWithThisId = std::find_if(entities.begin(), entities.end(),
				[component](const Entity& a) { return a.m_entityId == component->m_entityId; });
			if (entityWithThisId == entities.end())
			{
				Entity newEntity;
				newEntity.m_entityId = component->m_entityId;
				newEntity.m_components.insert({ component->GetComponentType(), component });
				entities.push_back(newEntity);
			}
			else
			{
				entityWithThisId->m_components.insert({ component->GetComponentType(), component });
			}
		}
	}
}

}
}