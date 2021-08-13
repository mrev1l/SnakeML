// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "IComponent.h"

namespace snakeml
{

class ComponentsPool
{
public:
	ComponentsPool() = default;
	~ComponentsPool();

	template<class ComponentT>
	void InsertComponents(Iterator* it);

	Iterator* GetComponents(ComponentType type) const;

	template<class ComponentsIterator>
	ComponentsIterator* GetComponents() const;

private:
	void DeleteComponents();

	std::unordered_map<size_t, Iterator*> m_componentsPool;
};

template<class ComponentT>
inline void ComponentsPool::InsertComponents(Iterator* it)
{
	const std::type_info& tInfo = typeid(ComponentT);
	const size_t hashCode = tInfo.hash_code();
	ASSERT(!m_componentsPool.contains(hashCode), "Inserting into an existing component pool.");
	m_componentsPool.insert({ hashCode, it });
}

template<class ComponentsIterator>
inline ComponentsIterator* snakeml::ComponentsPool::GetComponents() const
{
	const std::type_info& tInfo = typeid(ComponentsIterator);
	if (Iterator* it = m_componentsPool.at(tInfo.hash_code()))
	{
		return it->As<ComponentsIterator>();
	}
	return nullptr;
}

}
