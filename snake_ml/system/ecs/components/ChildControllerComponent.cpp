// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "ChildControllerComponent.h"

#include "utils/rapidjson_utils.h"

namespace snakeml
{

void ChildControllerComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	ChildControllerComponentIterator& container = *it->As<ChildControllerComponentIterator>();
	ChildControllerComponent& controller = container.Add();

	AttachComponentToEntity(controller, entity);
}

}