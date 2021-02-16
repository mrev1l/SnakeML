// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "RotateCubeSystem.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <chrono>

namespace snakeml
{
namespace system
{

void RotateCubeSystem::Execute()
{
	constexpr float angle = 0.16f;
	const uint32_t entityIdToUpdate = 0;

	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	const std::vector<Entity>::const_iterator entityIt = std::find_if(entities.begin(), entities.end(),
		[entityIdToUpdate](const Entity& a) {return a.m_entityId == entityIdToUpdate; });
	if (entityIt != entities.end())
	{
		TransformComponent& transform = *(TransformComponent*)(entityIt->m_components.at(ComponentType::TransformComponent));
		transform.m_rotation.m_x += angle;
		transform.m_rotation.m_y += angle;
		transform.m_rotation.m_z += angle;
	}
}

}
}