// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ParentSystem.h"

#include "system/ecs/components/ParentComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"

#include "system/ecs/ECSManager.h"

namespace snakeml
{

void ParentSystem::Update(float dt)
{
	ParentComponentIterator& parentComponentContainer = *ECSManager::GetInstance()->GetComponents<ParentComponentIterator>();
	for (ParentComponent& parentComponent : parentComponentContainer)
	{
		Entity& parent = ECSManager::GetInstance()->GetEntity(parentComponent.m_entityId);
		PhysicsComponent& parentPhysics = *parent.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();

		if (!IsNearlyZero((parentComponent.m_parentPreviousFramePosition - parentComponent.m_parentPreviousMovementTickPosition).length()) &&
			!IsNearlyZero((parentComponent.m_parentPreviousFramePosition - parentPhysics.m_position).length()))
		{
			parentComponent.m_parentPreviousMovementTickPosition = parentComponent.m_parentPreviousFramePosition;
			parentComponent.m_parentPreviousMovementTickRotation = parentComponent.m_parentPreviousFrameRotation;
		}

		/*if (!IsNearlyZero((parentComponent.m_parentPreviousFrameRotation - parentComponent.m_parentPreviousMovementTickRotation).length()) &&
			!IsNearlyZero((parentComponent.m_parentPreviousFrameRotation - parentPhysics.m_rotation).length()))
		{
			parentComponent.m_parentPreviousMovementTickRotation = parentComponent.m_parentPreviousFrameRotation;
		}*/

		if (!IsNearlyZero((parentComponent.m_parentPreviousFramePosition - parentPhysics.m_position).length()))
		{
			if (parentComponent.m_childId != -1)
			{
				Entity& child = ECSManager::GetInstance()->GetEntity(parentComponent.m_childId);
				PhysicsComponent& childPhysics = *child.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();
				TransformComponent& childTransform = *child.m_components.at(ComponentType::TransformComponent)->As<TransformComponent>();

				childPhysics.m_position = parentComponent.m_parentPreviousFramePosition;
				childTransform.m_position = childPhysics.m_position; // TODO : Fix
			}

			parentComponent.m_parentPreviousFramePosition = parentPhysics.m_position;
			parentComponent.m_parentPreviousFrameRotation = parentPhysics.m_rotation;
		}

		/*if (!IsNearlyZero((parentComponent.m_parentPreviousFrameRotation - parentPhysics.m_rotation).length()))
		{
			parentComponent.m_parentPreviousFrameRotation = parentPhysics.m_rotation;
		}*/
	}
}

}