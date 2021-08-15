// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "IComponent.h"
#include "Entity.h"

namespace snakeml
{

class EntityComponentPool
{
public:
	EntityComponentPool() = default;
	~EntityComponentPool();

	template<class ConcreteIteratorType>
	void InsertComponents(Iterator* it);

	template<class ComponentsIterator>
	ComponentsIterator* GetComponents() const;

	std::unordered_map<uint32_t, Entity>& GetEntities();
	Entity& GetEntity(uint32_t entityId);

private:
	template<class ComponentsIterator>
	void InsertComponents_AttachComponentsToEntities(Iterator* it);
	void DeleteComponents();

	std::unordered_map<size_t, Iterator*> m_componentsPool;
	std::unordered_map<uint32_t, Entity> m_entitiesPool;
};

#include "EntityComponentPool.inl"

}
