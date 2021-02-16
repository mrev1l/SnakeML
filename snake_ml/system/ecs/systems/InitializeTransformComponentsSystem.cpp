// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeTransformComponentsSystem.h"

#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{
namespace system
{

void InitializeTransformComponentsSystem::Execute()
{
	TransformComponentIterator* it = (TransformComponentIterator*)IComponent::CreateIterator(ComponentType::TransformComponent, 1);
	TransformComponent* material = (TransformComponent*)it->At(0);
	material->m_entityId = 0;
	material->m_position = { 0.f, 0.f, 0.f, 1.f };
	material->m_rotation = { 0.f, 0.f, 0.f, 0.f };
	material->m_scale = { 1.f, 1.f, 1.f, 0.f };

	ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::TransformComponent, it);
}

}
}