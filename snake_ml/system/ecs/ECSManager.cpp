// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ECSManager.h"

namespace snakeml
{

std::vector<Entity>& ECSManager::GetEntities()
{
	return m_entityComponentPool.GetEntities();
}

Entity* ECSManager::GetEntity(uint32_t id)
{
	std::vector<Entity>& entities = m_entityComponentPool.GetEntities();
	auto entityIt = std::find_if(entities.begin(), entities.end(), [id](const Entity& entity) { return entity.m_entityId == id; });
	if (entityIt != entities.end())
	{
		return entityIt._Ptr;
	}
	return nullptr;
}

void ECSManager::ExecuteSystem(const std::unique_ptr<ISystem>& system) const
{
	m_systemsPool.ExecuteSystem(system);
}

void ECSManager::ScheduleSystem(std::unique_ptr<ISystem>&& system)
{
	m_systemsPool.ScheduleSystem(std::forward<std::unique_ptr<ISystem>&&>(system));
}

void ECSManager::UnscheduleSystem(const ISystem* system)
{
	m_systemsPool.UnscheduleSystem(system);
}

void ECSManager::Update(double deltaTime)
{
	m_systemsPool.Update(deltaTime);
}

}
