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
	if (Iterator* it = m_componentsPool.at(tInfo.hash_code()))
	{
		return it->As<ComponentsIterator>();
	}
	return nullptr;
}

template<class ComponentsIterator>
inline void EntityComponentPool::InsertComponents_AttachComponentsToEntities(Iterator* it)
{
	std::vector<Entity>& entitiesRef = m_entities;
	const auto AttachComponentToEntity = [&entitiesRef](IComponent* component) -> void
	{
		std::vector<Entity>::iterator entityWithThisId = std::find_if(entitiesRef.begin(), entitiesRef.end(),
			[component](const Entity& a) { return a.m_entityId == component->m_entityId; });

		if (entityWithThisId == entitiesRef.end())
		{
			Entity newEntity;
			newEntity.m_entityId = component->m_entityId;
			newEntity.m_components.insert({ component->GetComponentType(), component });
			entitiesRef.push_back(newEntity);
		}
		else
		{
			entityWithThisId->m_components.insert({ component->GetComponentType(), component });
		}
	};

	ComponentsIterator* concreteIterator = it->As<ComponentsIterator>();
	ASSERT(concreteIterator, "[EntityComponentPool::InsertComponents] Missconfigured input paramter.");
	ComponentsIterator& iteratorRef = *concreteIterator;
	for (auto& component : iteratorRef)
	{
		AttachComponentToEntity(&component);
	}
}