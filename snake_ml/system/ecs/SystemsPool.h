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

	template<class SystemT>
	SystemT* GetSystem() const;

private:
	std::vector<std::unique_ptr<ISystem>> m_systems;
	std::unordered_set<uint32_t> m_systemsToUnschedule;
};

template<class SystemT>
inline SystemT* SystemsPool::GetSystem() const
{
	for (const std::unique_ptr<ISystem>& system : m_systems)
	{
		SystemT* result = system->As<SystemT>();
		if (result)
		{
			return result;
		}
	}
	return nullptr;
}

}