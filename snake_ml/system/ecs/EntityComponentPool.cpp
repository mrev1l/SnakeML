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

void EntityComponentPool::DeleteComponents()
{
	for (auto& component : m_componentsPool)
	{
		component.second->Clear();
	}
}

}
