// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "IComponent.h"

namespace snakeml
{
namespace system
{

class ComponentsPool
{
public:
	ComponentsPool() = default;

	void InsertComponents(ComponentType type, Iterator* it)
	{
		ASSERT(!m_componentsPool.contains(type), "Inserting into an existing component pool");
		m_componentsPool.insert({type, it});
	}
	Iterator* GetComponents(ComponentType type) const
	{
		return m_componentsPool.at(type);
	}

private:
	std::unordered_map<ComponentType, Iterator*> m_componentsPool;
};

}
}