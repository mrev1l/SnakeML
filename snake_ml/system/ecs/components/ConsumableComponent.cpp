// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ConsumableComponent.h"

namespace snakeml
{

void ConsumableComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	ConsumableComponentIterator& container = *it->As<ConsumableComponentIterator>();

	ConsumableComponent& consumable = container.Add();

	consumable.m_entityId = entity.m_entityId;
	entity.m_components.insert({ ComponentType::ConsumableComponent, &consumable });
}

}