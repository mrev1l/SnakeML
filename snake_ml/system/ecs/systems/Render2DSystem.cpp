// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "Render2DSystem.h"

#include "system/drivers/RenderDriver.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{

void Render2DSystem::Update(float deltaTime)
{
	IRenderDriver* renderDriver = IRenderDriver::GetInstance();
	const std::unordered_map<uint32_t, Entity>& entities = ECSManager::GetInstance()->GetEntities();
	for (const std::pair<uint32_t, Entity>& entity : entities)
	{
		if (entity.second.m_components.contains(ComponentType::DX12RenderComponent) || entity.second.m_components.contains(ComponentType::DebugRenderComponent))
		{
			renderDriver->SubscribeForRendering(entity.second);
		}
	}
}

}
