// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InputDataComponent.h"

namespace snakeml
{

void InputDataComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	InputDataComponentIterator& container = *it->As<InputDataComponentIterator>();

	InputDataComponent& idc = container.Add();

	idc.m_entityId = entity.m_entityId;
	entity.m_components.insert({ ComponentType::InputDataComponent, &idc });
}

}