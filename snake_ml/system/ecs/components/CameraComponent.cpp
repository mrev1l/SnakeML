// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "CameraComponent.h"

namespace snakeml
{

void CameraComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	CameraComponentIterator& container = *it->As<CameraComponentIterator>();

	CameraComponent& camera = container.Add();

	camera.m_entityId = entity.m_entityId;

	ParseVectorValue(m_description, "eyePosition", camera.m_eyePosition);
	ParseVectorValue(m_description, "focusPoint", camera.m_focusPoint);
	ParseVectorValue(m_description, "upDirection", camera.m_upDirection);

	entity.m_components.insert({ ComponentType::CameraComponent, &camera });
}

void CameraComponentConstructionVisitor::ParseVectorValue(const rapidjson::Value& json, const char* name, vector& _outVector)
{
	ASSERT(json.HasMember(name) && json[name].IsArray() && json[name].Size() == 3u, "[CameraComponentConstructionVisitor::ParseVectorValue] : Invalid camera json");

	const rapidjson::GenericArray<true, rapidjson::Value>& vectorJson = json[name].GetArray();

	_outVector = { vectorJson[0].GetFloat(), vectorJson[1].GetFloat(), vectorJson[2].GetFloat() };
}

}