// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "SelectSnakeSegmentTextureIdSystem.h"

#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/ParentComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/MaterialComponent.h"

#include "system/ecs/ECSManager.h"

namespace snakeml
{

// Only for pairs of std::hash-able types for simplicity.
// You can of course template this struct to allow other hash functions
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);

		// Mainly for demonstration purposes, i.e. works but is overly simple
		// In the real world, use sth. like boost.hash_combine
		return h1 ^ h2;
	}
};

void SelectSnakeSegmentTextureIdSystem::Update(float dt)
{
	struct cache
	{
		float headYaw;
		float secondYaw;
		MaterialComponent::TextureId secondId;
	};
	static std::vector<cache> s_calcCache;


	std::vector<Entity*> snakeSegments; // TODO try std reference_wrapper

	EntityControllerComponentIterator* snakeHeadContainer = ECSManager::GetInstance()->GetComponents<EntityControllerComponentIterator>();
	ASSERT(snakeHeadContainer->Size() == 1, "[SelectSnakeSegmentTextureIdSystem::Update] : Unexpected number of snake heads.");

	snakeSegments.push_back(&ECSManager::GetInstance()->GetEntity(snakeHeadContainer->At(0).m_entityId));

	uint32_t childId = -1;
	ASSERT(snakeSegments.back()->m_components.contains(ComponentType::ParentComponent), "[SelectSnakeSegmentTextureIdSystem::Update] : Misconfigured parent entity.");
	childId = snakeSegments.back()->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_childId;

	while (childId != -1)
	{
		snakeSegments.push_back(&ECSManager::GetInstance()->GetEntity(childId));

		ASSERT(snakeSegments.back()->m_components.contains(ComponentType::ParentComponent), "[SelectSnakeSegmentTextureIdSystem::Update] : Misconfigured parent entity.");
		childId = snakeSegments.back()->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_childId;
	}
	
	if (snakeSegments.size() == 1)
	{
		return;
	}

	// 0 segment
	const vector snakeHeadPos = snakeSegments.front()->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position;
	if (!IsNearlyZero((snakeHeadPos - m_oldHeadPosition).length()))
	{
		m_hasHeadMoved = true;
		m_oldHeadPosition = snakeHeadPos;
	}
	if (!m_hasHeadMoved)
	{
		return;
	}

	snakeSegments.front()->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::Head0;

	if (snakeSegments.front()->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z != 0.f)
	{
		int stop = 45;
	}
	enum SegmentDirection
	{
		Left, Right, Up, Down, Invalid
	};

	// New take 2

	auto SelectTextureId = [](Entity* segment, SegmentDirection headDirection, SegmentDirection headOldDirection) -> void
	{
		if (headDirection == Right && headOldDirection == Right)
		{
			segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
			segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = -90.f;
			segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = -90.f;
		}
		else if (headDirection == Left && headOldDirection == Left)
		{
			segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
			segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 90.f;
			segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 90.f;
		}
		else if (headDirection == Up && headOldDirection == Up)
		{
			segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
			segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 0.f;
			segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 0.f;
		}
		else if (headDirection == Down && headOldDirection == Down)
		{
			segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::StraightVertical;
			segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 180.f;
			segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 180.f;
		}
		else
		{
			if (headDirection == Up && headOldDirection == Right)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 0.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 0.f;
			}
			else if (headDirection == Right && headOldDirection == Down)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = -90.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = -90.f;
			}
			else if (headDirection == Down && headOldDirection == Left)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 180.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 180.f;
			}
			else if (headDirection == Left && headOldDirection == Up)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::LeftUp;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 90.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 90.f;
			}
			else if (headDirection == Up && headOldDirection == Left)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 0.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 0.f;
			}
			else if (headDirection == Left && headOldDirection == Down)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 90.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 90.f;
			}
			else if (headDirection == Down && headOldDirection == Right)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = 180.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = 180.f;
			}
			else if (headDirection == Right && headOldDirection == Up)
			{
				segment->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::DownRight;
				segment->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = -90.f;
				segment->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = -90.f;
			}
		}
	};

	for (uint32_t i = snakeSegments.size() - 1; i > 0; --i)
	{
		Entity* parent = snakeSegments[i - 1];
		Entity* current = snakeSegments[i];

		const vector headRotation = parent->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation;
		const SegmentDirection headDirection =
			IsNearlyEqual(headRotation.z, 0.f) ? Up :
			IsNearlyEqual(headRotation.z, 180.f) ? Down :
			IsNearlyEqual(headRotation.z, 90.f) ? Left :
			IsNearlyEqual(headRotation.z, -90.f) ? Right :
			Invalid;

		if (headDirection == Invalid)
		{
			int stop = 34;
		}

		const vector headOldRotation = parent->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_parentPreviousMovementTickRotation;
		const SegmentDirection headOldDirection =
			IsNearlyEqual(headOldRotation.z, 0.f) ? Up :
			IsNearlyEqual(headOldRotation.z, 180.f) ? Down :
			IsNearlyEqual(headOldRotation.z, 90.f) ? Left :
			IsNearlyEqual(headOldRotation.z, -90.f) ? Right :
			Invalid;

		if (headOldDirection == Invalid)
		{
			int stop = 34;
		}

		SelectTextureId(current, headDirection, headOldDirection);
		if (i == 2)
		{
			//s_calcCache.push_back({
			//	headRotation.z,
			//	current->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z,
			//	current->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId });
			//if (false)
			//{
			//	s_calcCache.clear();
			//}
		}
	}
	m_hasHeadMoved = false;
}

}