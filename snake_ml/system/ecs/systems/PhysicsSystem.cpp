// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "PhysicsSystem.h"

#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

namespace snakeml
{
namespace system
{

void PhysicsSystem::Update(double deltaTime)
{
	PhysicsComponentIterator* physicsComponents =  (PhysicsComponentIterator*)ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::PhysicsComponent);
	for (size_t i = 0u; i < physicsComponents->Size(); ++i)
	{
		PhysicsComponent& body = *((PhysicsComponent*)physicsComponents->At(i));
		UpdateAABB(body);
	}
}

void PhysicsSystem::UpdateAABB(PhysicsComponent& body)
{
	std::array<math::vector, 4> boundingBox =
	{
		math::vector{body.m_position.x - body.m_shape.m_dimensions.x / 2.f, body.m_position.y - body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{body.m_position.x - body.m_shape.m_dimensions.x / 2.f, body.m_position.y + body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{body.m_position.x + body.m_shape.m_dimensions.x / 2.f, body.m_position.y - body.m_shape.m_dimensions.y / 2.f, 0.f},
		math::vector{body.m_position.x + body.m_shape.m_dimensions.x / 2.f, body.m_position.y + body.m_shape.m_dimensions.y / 2.f, 0.f}
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

}
}