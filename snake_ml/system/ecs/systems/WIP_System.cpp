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
	static bool s_update = true;
	static constexpr float k_physicsTimeStep = 1.f / 144.f; // 1 simulation step per frame per 144 fps

	float timeToSimulate = deltaTime;
	static math::vector force = math::vector{ 1.0f, 0.5f, 0.f };
	do
	{
		const float dt = timeToSimulate > k_physicsTimeStep ? k_physicsTimeStep : timeToSimulate;
		timeToSimulate -= dt;

		const uint32_t entityIdToUpdate = 0;
		constexpr float scale = 0.1f;

		const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
		const std::vector<Entity>::const_iterator entityIt = std::find_if(entities.begin(), entities.end(),
			[entityIdToUpdate](const Entity& a) {return a.m_entityId == entityIdToUpdate; });
		if (entityIt != entities.end())
		{
			PhysicsComponent& body = *(PhysicsComponent*)entityIt->m_components.at(ComponentType::PhysicsComponent);

			// Linear movement integration
			if (s_update)
			{
				//const math::vector force = math::vector{ 1.f, 1.f, 0.f };// *1000000.f;
				math::vector accDelta = (force * -1.f * 10000.f / body.m_shape.m_mass) * dt;
				body.m_acceleration = accDelta;
				s_update = false;
			}
		}
	}
	while (!math::IsNearlyZero(timeToSimulate, math::k_default_epsilon));
}

}
}
}