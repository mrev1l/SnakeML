// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ComponentsPool.h"

namespace snakeml
{

ComponentsPool::~ComponentsPool()
{
	DeleteComponents();
}

Iterator* ComponentsPool::GetComponents(ComponentType type) const
{
	for (auto componentIt : m_componentsPool2)
	{
		ASSERT(componentIt.second->Size(), "[Components Pool] : Missconfigured component iterator.");
		if (componentIt.second->GetInterfacePtr(0)->GetComponentType() == type)
		{
			return componentIt.second;
		}
	}
	return nullptr;
}

void ComponentsPool::DeleteComponents()
{
	for (auto& component : m_componentsPool2)
	{
		IComponent::DeleteIterator(component.second->GetInterfacePtr(0)->GetComponentType(), component.second);
	}
}

}
