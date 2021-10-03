// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "TransformComponent.h"

namespace snakeml
{

void TransformComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	TransformComponentIterator& container = *it->As<TransformComponentIterator>();

	TransformComponent& transform = container.Add();

	ParseTransformComponent_Position(m_description, transform.m_position);
	ParseTransformComponent_Rotation(m_description, transform.m_rotation);
	ParseTransformComponent_Scale(m_description, transform.m_scale);

	transform.m_entityId = entity.m_entityId;
	entity.m_components.insert({ ComponentType::TransformComponent, &transform });
}

void TransformComponentConstructionVisitor::ParseTransformComponent_Position(const rapidjson::Value& json, vector& _outPosition)
{
	ASSERT(json.HasMember("position") && json["position"].IsArray() && json["position"].Size() == 3u, "Invalid position json");

	const rapidjson::GenericArray<true, rapidjson::Value>& positionJson = json["position"].GetArray();

	_outPosition = { positionJson[0].GetFloat(), positionJson[1].GetFloat(), positionJson[2].GetFloat() };
}

void TransformComponentConstructionVisitor::ParseTransformComponent_Rotation(const rapidjson::Value& json, vector& _outRotation)
{
	ASSERT(json.HasMember("rotation") && json["rotation"].IsArray() && json["rotation"].Size() == 3u, "Invalid rotation json");

	const rapidjson::GenericArray<true, rapidjson::Value>& rotationJson = json["rotation"].GetArray();

	_outRotation = { rotationJson[0].GetFloat(), rotationJson[1].GetFloat(), rotationJson[2].GetFloat() };
}

void TransformComponentConstructionVisitor::ParseTransformComponent_Scale(const rapidjson::Value& json, vector& _outScale)
{
	ASSERT(json.HasMember("scale") && json["scale"].IsArray() && json["scale"].Size() == 3u, "Invalid scale json");

	const rapidjson::GenericArray<true, rapidjson::Value>& scaleJson = json["scale"].GetArray();

	_outScale = { scaleJson[0].GetFloat(), scaleJson[1].GetFloat(), scaleJson[2].GetFloat() };
}

}