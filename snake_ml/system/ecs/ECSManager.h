// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "ComponentsPool.h"
#include "Entity.h"
#include "ISystem.h"

#include "utils/patterns/singleton.h"

namespace snakeml
{
namespace system
{

class ECSManager : public patterns::Singleton<ECSManager>
{
public:
	ECSManager() = default;
	~ECSManager() = default;

	inline ComponentsPool& GetComponentsPool() { return m_components; }
	inline std::vector<Entity>& GetEntities() { return m_entities; }

	void ExecuteSystem(ISystem* system);
	void ScheduleSystem(ISystem* system);

	void Update();

private:
	ComponentsPool m_components;
	std::vector<Entity> m_entities;
	std::vector<ISystem*> m_systems;
};

}
}