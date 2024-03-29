// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeCameraSystem.h"

#include "system/ecs/components/CameraComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{
// todo rework into init level system
void InitializeCameraSystem::Execute()
{
	CameraComponentIterator* it = IComponent::CreateIterator(ComponentType::CameraComponent, 1)->As<CameraComponentIterator>();
	CameraComponent& camera = it->At(0);

	camera.m_entityId = 1;
	camera.m_eyePosition =	{ 0.f, 0.f, -10.f };
	camera.m_focusPoint =	{ 0.f, 0.f, 0.f };
	camera.m_upDirection =	{ 0.f, 1.f, 0.f };

	ECSManager::GetInstance()->InsertComponents<CameraComponentIterator>(it);
}

}
