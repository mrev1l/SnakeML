// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "PhysicsComponent.h"

#include "utils/rapidjson_utils.h"

namespace snakeml
{

void PhysicsComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	PhysicsComponentIterator& container = *it->As<PhysicsComponentIterator>();

	PhysicsComponent& physicsComp = container.Add();

	RapidjsonUtils::ParseVectorValue(m_description, k_shapeDimensionsValueName, physicsComp.m_shape.m_dimensions);
	RapidjsonUtils::ParseFloatValue(m_description, k_shapeMassValueName, physicsComp.m_shape.m_mass);
	RapidjsonUtils::ParseBoolValue(m_description, k_isDynamicValueName, physicsComp.m_isDynamic);
	RapidjsonUtils::ParseEnumValue<CollisionChannel>(m_description, k_collisionChannelValueName, physicsComp.m_collisionChannel);
	RapidjsonUtils::ParseEnumValue<CollisionChannel>(m_description, k_collisionFilterValueName, physicsComp.m_collisionFilter);

	AttachComponentToEntity(physicsComp, entity);
}

}