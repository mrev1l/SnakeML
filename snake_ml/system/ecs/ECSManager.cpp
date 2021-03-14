// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ECSManager.h"

namespace snakeml
{
namespace system
{

void ECSManager::ExecuteSystem(const std::unique_ptr<ISystem>& system)
{
	system->Execute();
}

void ECSManager::ScheduleSystem(std::unique_ptr<ISystem>&& system)
{
	m_systems.emplace_back(std::move(system));
}

void ECSManager::Update()
{
	for (const std::unique_ptr<ISystem>& system : m_systems)
	{
		system->Execute();
	}
}

}
}
