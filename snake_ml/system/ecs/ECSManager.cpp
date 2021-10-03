// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ECSManager.h"

namespace snakeml
{

Iterator* ECSManager::GetComponents(ComponentType type) const
{
	return m_entityComponentPool.GetComponents(type);
}

std::unordered_map<uint32_t, Entity>& ECSManager::GetEntities()
{
	return m_entityComponentPool.GetEntities();
}

Entity& ECSManager::GetEntity(uint32_t id)
{
	return m_entityComponentPool.GetEntity(id);
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

void ECSManager::Update(float deltaTime)
{
	m_systemsPool.Update(deltaTime);
}

}
