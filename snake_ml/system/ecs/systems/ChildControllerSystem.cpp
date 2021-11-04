// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ChildControllerSystem.h"

#include "system/ecs/components/ChildControllerComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

namespace snakeml
{

void ChildControllerSystem::Update(float dt)
{
	ChildControllerComponentIterator& childControllers = *ECSManager::GetInstance()->GetComponents<ChildControllerComponentIterator>();
	for (ChildControllerComponent& childController : childControllers)
	{
		Entity& parent = ECSManager::GetInstance()->GetEntity(childController.m_parentId);
		PhysicsComponent& parentPhysics = *parent.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();

		Entity& child = ECSManager::GetInstance()->GetEntity(childController.m_entityId);
		PhysicsComponent& childPhysics = *child.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();
		TransformComponent& childTransform = *child.m_components.at(ComponentType::TransformComponent)->As<TransformComponent>();

		if (!IsNearlyZero((childController.m_parentPosition - parentPhysics.m_position).length()))
		{
			childController.m_parentPreviousPosition = childController.m_parentPosition;
			childController.m_parentPosition = parentPhysics.m_position;

			childPhysics.m_position = childController.m_parentPreviousPosition;
			childTransform.m_position = childPhysics.m_position; // TODO : Fix
		}

		//if (!IsNearlyZero((childController.m_parentRotation - parentPhysics.m_rotation).length()))
		//{
		//	childController.m_parentPreviousRotation = childController.m_parentRotation;
		//	childController.m_parentRotation = parentPhysics.m_rotation;

		//	childPhysics.m_rotation = childController.m_parentRotation;
		//	childTransform.m_rotation = childPhysics.m_rotation;// TODO : Fix
		//}
	}

}

}