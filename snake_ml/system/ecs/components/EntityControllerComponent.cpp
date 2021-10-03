// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "EntityControllerComponent.h"

namespace snakeml
{

void EntityControllerComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	EntityControllerComponentIterator& container = *it->As<EntityControllerComponentIterator>();

	EntityControllerComponent& ec = container.Add();

	ec.m_entityId = entity.m_entityId;
	entity.m_components.insert({ ComponentType::EntityControllerComponent, &ec });
}

}