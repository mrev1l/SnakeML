// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "Render2DSystem.h"

#include "system/drivers/RenderDriver.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{
namespace system
{

void Render2DSystem::Execute()
{
	IRenderDriver* renderDriver = IRenderDriver::GetInstance();
	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	for (const Entity& entity : entities)
	{
		if (entity.m_components.contains(ComponentType::DX12RenderComponent))
		{
			renderDriver->SubscribeForRendering(entity);
		}
	}
}

}
}