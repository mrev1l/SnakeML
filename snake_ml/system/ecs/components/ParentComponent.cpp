// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ParentComponent.h"

namespace snakeml
{

void ParentComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	ParentComponentIterator& container = *it->As<ParentComponentIterator>();
	ParentComponent& component = container.Add();

	AttachComponentToEntity(component, entity);
}

}