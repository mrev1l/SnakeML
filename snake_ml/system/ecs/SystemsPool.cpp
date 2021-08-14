// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "SystemsPool.h"

namespace snakeml
{

void SystemsPool::ExecuteSystem(const std::unique_ptr<ISystem>& system) const
{
	system->Execute();
}

void SystemsPool::ScheduleSystem(std::unique_ptr<ISystem>&& system)
{
	m_systems.emplace_back(std::move(system));
}

void SystemsPool::UnscheduleSystem(const ISystem* system)
{
	ASSERT(system, "[ECSManager::UnscheduleSystem] : Empty system");
	m_systemsToUnschedule.insert(system->m_id);
}

void SystemsPool::Update(double deltaTime)
{
	for (const std::unique_ptr<ISystem>& system : m_systems)
	{
		system->Update(deltaTime);
	}

	for (const uint32_t id : m_systemsToUnschedule)
	{
		auto ShouldEraseSystem = [id](const std::unique_ptr<ISystem>& system) -> bool { return system->m_id == id; };
		m_systems.erase(std::remove_if(m_systems.begin(), m_systems.end(), ShouldEraseSystem));
	}
	m_systemsToUnschedule.clear();
}

}