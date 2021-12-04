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

const std::unordered_map<
	SelectSnakeSegmentTextureIdSystem::ParentSegmentTranslation,
	SelectSnakeSegmentTextureIdSystem::SegmentTextureId,
	SelectSnakeSegmentTextureIdSystem::pair_hash> SelectSnakeSegmentTextureIdSystem::k_parentSegmentMovementToTextureIdMap =
{
	{ { Right,	Right },	{ MaterialComponent::TextureId::StraightVertical,	-90.f } },
	{ { Left,	Left },		{ MaterialComponent::TextureId::StraightVertical,	90.f } },
	{ { Up,		Up },		{ MaterialComponent::TextureId::StraightVertical,	0.f } },
	{ { Down,	Down },		{ MaterialComponent::TextureId::StraightVertical,	180.f } },

	{ { Up,		Right },	{ MaterialComponent::TextureId::LeftUp,				0.f } },
	{ { Right,	Down },		{ MaterialComponent::TextureId::LeftUp,				-90.f } },
	{ { Down,	Left },		{ MaterialComponent::TextureId::LeftUp,				180.f } },
	{ { Left,	Up },		{ MaterialComponent::TextureId::LeftUp,				90.f } },

	{ { Up,		Left },		{ MaterialComponent::TextureId::DownRight,			0.f } },
	{ { Left,	Down },		{ MaterialComponent::TextureId::DownRight,			90.f } },
	{ { Down,	Right },	{ MaterialComponent::TextureId::DownRight,			180.f } },
	{ { Right,	Up },		{ MaterialComponent::TextureId::DownRight,			-90.f } }
};

void SelectSnakeSegmentTextureIdSystem::Update(float dt)
{
	std::vector<Entity*> snakeSegments = BuildSnakeSegmentsVector();
	if (snakeSegments.size() == 1)
	{
		return;
	}

	UpdateHasMovedToggle(snakeSegments[0]);
	if (!m_hasHeadMoved)
	{
		return;
	}

	snakeSegments.front()->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::Head0;

	for (uint32_t i = snakeSegments.size() - 1; i > 0; --i)
	{
		const Entity* parent = snakeSegments[i - 1];
		Entity* current = snakeSegments[i];

		const SegmentDirection headDirection = CalculateSegmentDirection(parent->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z);
		const SegmentDirection headOldDirection = CalculateSegmentDirection(parent->m_components.at(ComponentType::ParentComponent)->As<ParentComponent>()->m_parentPreviousMovementTickRotation.z);

		ASSERT(headDirection != Invalid, "[SelectSnakeSegmentTextureIdSystem::Update] : Unexpected segment yaw.");
		ASSERT(headOldDirection != Invalid, "[SelectSnakeSegmentTextureIdSystem::Update] : Unexpected segment yaw.");

		const SegmentTextureId& textureIdYaw = k_parentSegmentMovementToTextureIdMap.at({ headDirection, headOldDirection });
		current->m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = textureIdYaw.first;
		current->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_rotation.z = textureIdYaw.second;
		current->m_components.at(ComponentType::TransformComponent)->As<TransformComponent>()->m_rotation.z = textureIdYaw.second;
	}
	m_hasHeadMoved = false;
}

void SelectSnakeSegmentTextureIdSystem::UpdateHasMovedToggle(Entity* snakeHead)
{
	const vector snakeHeadPos = snakeHead->m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>()->m_position;
	if (!IsNearlyZero((snakeHeadPos - m_oldHeadPosition).length()))
	{
		m_hasHeadMoved = true;
		m_oldHeadPosition = snakeHeadPos;
	}
}

std::vector<Entity*> SelectSnakeSegmentTextureIdSystem::BuildSnakeSegmentsVector()
{
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

	return snakeSegments;
}

SelectSnakeSegmentTextureIdSystem::SegmentDirection SelectSnakeSegmentTextureIdSystem::CalculateSegmentDirection(float yaw)
{
	return IsNearlyEqual(yaw, 0.f) ? Up :
		IsNearlyEqual(yaw, 180.f) ? Down :
		IsNearlyEqual(yaw, 90.f) ? Left :
		IsNearlyEqual(yaw, -90.f) ? Right :
		Invalid;
}

}