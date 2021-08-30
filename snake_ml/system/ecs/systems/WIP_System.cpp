// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "WIP_System.h"

#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <chrono>
#include <iterator>

namespace snakeml
{
namespace wip
{

void WIP_System::Update(float deltaTime)
{
	// Give initial nudge
	const uint32_t entityIdToUpdate = 0;
	Entity& entity = ECSManager::GetInstance()->GetEntity(entityIdToUpdate);
	EntityControllerComponent& controller = *entity.m_components.at(ComponentType::EntityControllerComponent)->As<EntityControllerComponent>();

	controller.m_inputVector = vector::up;

	ECSManager::GetInstance()->UnscheduleSystem(this);
}

}
}
