// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "TransformComponent.h"

#include "utils/rapidjson_utils.h"

namespace snakeml
{

void TransformComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	TransformComponentIterator& container = *it->As<TransformComponentIterator>();

	TransformComponent& transform = container.Add();

	RapidjsonUtils::ParseVectorValue(m_description, k_positionValueName, transform.m_position);
	RapidjsonUtils::ParseVectorValue(m_description, k_rotationValueName, transform.m_rotation);
	RapidjsonUtils::ParseVectorValue(m_description, k_scaleValueName, transform.m_scale);

	AttachComponentToEntity(transform, entity);
}

}