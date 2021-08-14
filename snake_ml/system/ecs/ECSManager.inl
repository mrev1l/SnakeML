#pragma once

template<class ConcreteIteratorType>
inline void ECSManager::InsertComponents(Iterator* it)
{
	m_entityComponentPool.InsertComponents<ConcreteIteratorType>(it);
}

template<class ConcreteIteratorType>
inline ConcreteIteratorType* ECSManager::GetComponents() const
{
	return m_entityComponentPool.GetComponents<ConcreteIteratorType>();
}