// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "EntityComponentPool.h"

namespace snakeml
{

EntityComponentPool::~EntityComponentPool()
{
	DeleteComponents();
}

std::unordered_map<uint32_t, Entity>& EntityComponentPool::GetEntities()
{
	return m_entitiesPool;
}

Entity& EntityComponentPool::GetEntity(uint32_t entityId)
{
	ASSERT(m_entitiesPool.contains(entityId), "[EntityComponentPool::GetEntity] invalid entity id");
	return m_entitiesPool[entityId];
}

void EntityComponentPool::DeleteComponents()
{
	for (auto& component : m_componentsPool)
	{
		component.second->Clear();
	}
}

}
