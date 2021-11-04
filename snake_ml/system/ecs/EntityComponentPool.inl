#pragma once

template<class ConcreteIteratorType>
inline void EntityComponentPool::InsertComponents(Iterator* it)
{
	const std::type_info& tInfo = typeid(ConcreteIteratorType);
	const size_t hashCode = tInfo.hash_code();
	ASSERT(!m_componentsPool.contains(hashCode), "Inserting into an existing component pool.");

	InsertComponents_AttachComponentsToEntities<ConcreteIteratorType>(it);
	m_componentsPool.insert({ hashCode, it });
}

template<class ComponentsIterator>
inline ComponentsIterator* EntityComponentPool::GetComponents() const
{
	const std::type_info& tInfo = typeid(ComponentsIterator);
	if (!m_componentsPool.contains(tInfo.hash_code()))
	{
		return nullptr;
	}

	Iterator* it = m_componentsPool.at(tInfo.hash_code());
	return it->As<ComponentsIterator>();
}

template<class ComponentsIterator>
inline void EntityComponentPool::InsertComponents_AttachComponentsToEntities(Iterator* it)
{
	ComponentsIterator* concreteIterator = it->As<ComponentsIterator>();
	ASSERT(concreteIterator, "[EntityComponentPool::InsertComponents] Missconfigured input paramter.");
	ComponentsIterator& iteratorRef = *concreteIterator;
	for (auto& component : iteratorRef)
	{
		m_entitiesPool[component.m_entityId].m_components.insert({ component.GetComponentType(), &component });
	}
}