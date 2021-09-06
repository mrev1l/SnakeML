// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once
#include "EntityComponentPool.h"
#include "SystemsPool.h"

#include "utils/patterns/singleton.h"

namespace snakeml
{

class ECSManager : public Singleton<ECSManager>
{
public:
	ECSManager() = default;
	~ECSManager() = default;

	/** Relay to entity-components pool */
	template<class ConcreteIteratorType>
	void InsertComponents(Iterator* it);
	template<class ComponentsIterator>
	ComponentsIterator* GetComponents() const;

	std::unordered_map<uint32_t, Entity>& GetEntities();
	Entity& GetEntity(uint32_t id);

	/** Relay to systems pool */
	void ExecuteSystem(const std::unique_ptr<ISystem>& system) const;
	void ScheduleSystem(std::unique_ptr<ISystem>&& system);
	void UnscheduleSystem(const ISystem* system);

	void Update(float deltaTime);

	template<class SystemT>
	SystemT* GetSystem() const;

private:
	EntityComponentPool m_entityComponentPool;
	SystemsPool m_systemsPool;
};

#include "ECSManager.inl"

}
