// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "TestMoveSnakeHeadSystem.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <chrono>

namespace snakeml
{
namespace system
{

void TestMoveSnakeHeadSystem::Execute()
{
	const uint32_t entityIdToUpdate = 0;
	constexpr float scale = 0.1f;
	constexpr float screenW = 720.f, screenH = 720.f;
	static uint32_t cellIdx = 0;

	static double elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	auto t1 = clock.now(); //-V656
	auto deltaTime = t1 - t0;
	t0 = t1;
	elapsedSeconds += deltaTime.count() * 1e-9;
	if (elapsedSeconds > 0.1)
	{
		elapsedSeconds = 0.f;
		const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
		const std::vector<Entity>::const_iterator entityIt = std::find_if(entities.begin(), entities.end(),
			[entityIdToUpdate](const Entity& a) {return a.m_entityId == entityIdToUpdate; });
		if (entityIt != entities.end())
		{
			TransformComponent& transform = *(TransformComponent*)(entityIt->m_components.at(ComponentType::TransformComponent));
			transform.m_scale.m_x = scale;
			transform.m_scale.m_y = scale;

			float x = static_cast<float>(cellIdx % 10);
			x *= 72.f;
			x -= 360.f;
			x += 36.f;
			float y = static_cast<float>(cellIdx / 10);
			y *= 72.f;
			y -= 360.f;
			y *= -1;
			y -= 36.f;
			transform.m_position = { x, y };

			cellIdx = ++cellIdx % 100; //-V567
		}
	}
}

}
}