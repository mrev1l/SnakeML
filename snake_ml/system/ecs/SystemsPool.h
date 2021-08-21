#pragma once

#include "ISystem.h"

namespace snakeml
{

class SystemsPool
{
public:
	SystemsPool() = default;
	~SystemsPool() = default;

	void ExecuteSystem(const std::unique_ptr<ISystem>& system) const;
	void ScheduleSystem(std::unique_ptr<ISystem>&& system);
	void UnscheduleSystem(const ISystem* system);

	void Update(float deltaTime);

private:
	std::vector<std::unique_ptr<ISystem>> m_systems;
	std::unordered_set<uint32_t> m_systemsToUnschedule;
};

}