// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "WIP_System.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include <chrono>

namespace snakeml
{
namespace system
{

void WIP_System::Update(double deltaTime)
{
	static bool s_update = true;

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
			const math::vector force = { 1.f, -1.f, 0.f };
			math::vector accDelta = (force / body.m_shape.m_mass) * deltaTime;
			body.m_acceleration += accDelta;
			body.m_velocity += body.m_acceleration * deltaTime;
			body.m_position += body.m_velocity * deltaTime;

			// Angular movement integration
			const math::vector torque = math::vector::forward * 1000.f;
			body.m_angularAcceleration += (torque / body.m_shape.m_momentOfInertia) * deltaTime;
			body.m_angularVelocity += body.m_angularAcceleration * deltaTime;
			body.m_rotation += body.m_angularVelocity * deltaTime;

			TransformComponent& transform = *(TransformComponent*)(entityIt->m_components.at(ComponentType::TransformComponent));
			transform.m_scale.x = scale;
			transform.m_scale.y = scale;
			transform.m_position = body.m_position;
			transform.m_rotation = body.m_rotation;
		}
	}

	PhysicsComponentIterator* bodys = (PhysicsComponentIterator*)ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::PhysicsComponent);
	// Update AABBs
	for (size_t i = 0u; i < bodys->Size(); ++i)
	{
		PhysicsComponent& body = *(PhysicsComponent*)bodys->At(i);

		std::array<math::vector, 4> boundingBox =
		{
			math::vector{/*body.m_position.x*/ - body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ - body.m_shape.m_dimensions.y / 2.f, 0.f},
			math::vector{/*body.m_position.x*/ - body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ + body.m_shape.m_dimensions.y / 2.f, 0.f},
			math::vector{/*body.m_position.x*/ + body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ - body.m_shape.m_dimensions.y / 2.f, 0.f},
			math::vector{/*body.m_position.x*/ + body.m_shape.m_dimensions.x / 2.f, /*body.m_position.y*/ + body.m_shape.m_dimensions.y / 2.f, 0.f}
		};

		const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(body.m_rotation.y), math::ConvertToRadians(body.m_rotation.x), math::ConvertToRadians(body.m_rotation.z));
		const math::matrix translationMatrix = math::TranslationMatrix(body.m_position.x, body.m_position.y, body.m_position.z);

		const math::matrix transformMatrix = rotationMatrix * translationMatrix;
		
		for (auto& vertex : boundingBox)
		{
			vertex = transformMatrix * vertex;
		}

		body.m_aabb = { {FLT_MAX, FLT_MAX, 0.f}, {-FLT_MAX, -FLT_MAX, 0.f} };
		for (auto vertex : boundingBox)
		{
			body.m_aabb.min.x = std::min(body.m_aabb.min.x, vertex.x);
			body.m_aabb.min.y = std::min(body.m_aabb.min.y, vertex.y);
			body.m_aabb.max.x = std::max(body.m_aabb.max.x, vertex.x);
			body.m_aabb.max.y = std::max(body.m_aabb.max.y, vertex.y);
		}
	}

	// Test overlaps
	for (size_t i = 0u; i < bodys->Size(); ++i)
	{
		for (size_t j = 0u; j < bodys->Size(); ++j)
		{
			if (i == j)
			{
				continue;
			}

			const PhysicsComponent& bodyA = *((PhysicsComponent*)bodys->At(i));
			const PhysicsComponent& bodyB = *((PhysicsComponent*)bodys->At(j));

			const AABB& a = bodyA.m_aabb;
			const AABB& b = bodyB.m_aabb;

			float d1x = b.min.x - a.max.x;
			float d1y = b.min.y - a.max.y;
			float d2x = a.min.x - b.max.x;
			float d2y = a.min.y - b.max.y;

			if (d1x >= 0.f || d1y >= 0.f)
			{
				continue;
			}

			if (d2x >= 0.f || d2y >= 0.f)
			{
				continue;
			}

			s_update = false;
		}
	}

	int stop = 45;
}

}
}