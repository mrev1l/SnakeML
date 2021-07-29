// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "WIP_System.h"

#include "system/ecs/components/MeshComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <chrono>
#include <iterator>

namespace snakeml
{
namespace system
{
namespace wip
{

void WIP_System::Update(double deltaTime)
{
	// Give initial nudge
	const uint32_t entityIdToUpdate = 0;
	Entity* entityPtr = ECSManager::GetInstance()->GetEntity(entityIdToUpdate);
	if (entityPtr)
	{
		PhysicsComponent& body = *(PhysicsComponent*)entityPtr->m_components.at(ComponentType::PhysicsComponent);

		const math::vector force = math::vector{ 1.0f, 0.5f, 0.f };
		const math::vector accDelta = (force * -1.f * 1000.f / body.m_shape.m_mass) * deltaTime;
		body.m_acceleration += accDelta;
	}

	ECSManager::GetInstance()->UnscheduleSystem(this);
}

}
}
}