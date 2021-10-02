// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeCameraSystem.h"

#include "system/ecs/components/CameraComponent.h"
#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/InputDataComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{
// todo rework into init level system
void InitializeCameraSystem::Execute()
{
	{
		CameraComponentIterator* it = IComponent::CreateIterator(ComponentType::CameraComponent, 1)->As<CameraComponentIterator>();
		CameraComponent& camera = it->Add();

		camera.m_entityId = 1;
		camera.m_eyePosition =	{ 0.f, 0.f, -10.f };
		camera.m_focusPoint =	{ 0.f, 0.f, 0.f };
		camera.m_upDirection =	{ 0.f, 1.f, 0.f };

		ECSManager::GetInstance()->InsertComponents<CameraComponentIterator>(it);
	}

	{
		InputDataComponentIterator* it = IComponent::CreateIterator(ComponentType::InputDataComponent, 1)->As<InputDataComponentIterator>();
		InputDataComponent& inputDataComponent = it->Add();

		inputDataComponent.m_entityId = 1;

		ECSManager::GetInstance()->InsertComponents<InputDataComponentIterator>(it);
	}

	{
		EntityControllerComponentIterator* it = IComponent::CreateIterator(ComponentType::EntityControllerComponent, 1)->As<EntityControllerComponentIterator>();
		EntityControllerComponent& controllerComponent = it->Add();

		controllerComponent.m_entityId = 0;
		controllerComponent.m_inputVector = vector::zero;

		ECSManager::GetInstance()->InsertComponents<EntityControllerComponentIterator>(it);
	}
}

}
