// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "PhysicsComponent.h"

namespace snakeml
{

void PhysicsComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	PhysicsComponentIterator& container = *it->As<PhysicsComponentIterator>();

	PhysicsComponent& physicsComp = container.Add();

	ParsePhysicsComponents_ShapeDimensions(m_description, physicsComp.m_shape.m_dimensions);
	ParsePhysicsComponents_ShapeMass(m_description, physicsComp.m_shape.m_mass);
	ParsePhysicsComponents_IsDynamic(m_description, physicsComp.m_isDynamic);
	ParsePhysicsComponents_CollisionChannel(m_description, physicsComp.m_collisionChannel);
	ParsePhysicsComponents_CollisionFilter(m_description, physicsComp.m_collisionFilter);

	physicsComp.m_entityId = entity.m_entityId;
	entity.m_components.insert({ ComponentType::PhysicsComponent, &physicsComp });
}

void PhysicsComponentConstructionVisitor::ParsePhysicsComponents_ShapeDimensions(const rapidjson::Value& json, vector& _outShapeDimensions)
{
	ASSERT(json.HasMember("shape_dimensions") && json["shape_dimensions"].IsArray() && json["shape_dimensions"].Size() == 3u, "Invalid shape dimension json");

	const rapidjson::GenericArray<true, rapidjson::Value>& shapeDimensionsJson = json["shape_dimensions"].GetArray();

	_outShapeDimensions = { shapeDimensionsJson[0].GetFloat(), shapeDimensionsJson[1].GetFloat(), shapeDimensionsJson[2].GetFloat() };
}

void PhysicsComponentConstructionVisitor::ParsePhysicsComponents_ShapeMass(const rapidjson::Value& json, float& _outMass)
{
	ASSERT(json.HasMember("shape_mass") && json["shape_mass"].IsFloat(), "Invalid shape mass json");

	_outMass = json["shape_mass"].GetFloat();
}

void PhysicsComponentConstructionVisitor::ParsePhysicsComponents_IsDynamic(const rapidjson::Value& json, bool& _outIsDynamic)
{
	ASSERT(json.HasMember("is_dynamic") && json["is_dynamic"].IsBool(), "Invalid is dynamic json");

	_outIsDynamic = json["is_dynamic"].GetBool();
}

void PhysicsComponentConstructionVisitor::ParsePhysicsComponents_CollisionChannel(const rapidjson::Value& json, CollisionChannel& _outCollisionChannel)
{
	ASSERT(json.HasMember("collision_channel") && json["collision_channel"].IsUint(), "Invalid collision channel json");

	const uint32_t collisionChannelData = json["collision_channel"].GetUint();
	_outCollisionChannel = static_cast<CollisionChannel>(collisionChannelData);
}

void PhysicsComponentConstructionVisitor::ParsePhysicsComponents_CollisionFilter(const rapidjson::Value& json, CollisionChannel& _outCollisionFilter)
{
	ASSERT(json.HasMember("collision_filter") && json["collision_filter"].IsUint(), "Invalid collision filter json");

	const uint32_t collisionChannelData = json["collision_filter"].GetUint();
	_outCollisionFilter = static_cast<CollisionChannel>(collisionChannelData);
}

}