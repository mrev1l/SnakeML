// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ECSManager.h"

namespace snakeml
{
namespace system
{

ECSManager::ECSManager()
{
}

void ECSManager::ExecuteSystem(ISystem* system)
{
	system->Execute();
}

void ECSManager::ScheduleSystem(ISystem* system)
{
	m_systems.push_back(system);
}

void ECSManager::Update()
{
	for (ISystem* system : m_systems)
	{
		system->Execute();
	}
}



}
}
