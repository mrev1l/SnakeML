// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "CameraComponent.h"

#include "utils/rapidjson_utils.h"

namespace snakeml
{

void CameraComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	CameraComponentIterator& container = *it->As<CameraComponentIterator>();
	CameraComponent& camera = container.Add();

	RapidjsonUtils::ParseVectorValue(m_description, k_eyePositionValueName, camera.m_eyePosition);
	RapidjsonUtils::ParseVectorValue(m_description, k_focusPointValueName, camera.m_focusPoint);
	RapidjsonUtils::ParseVectorValue(m_description, k_upDirectionValueName, camera.m_upDirection);

	AttachComponentToEntity(camera, entity);
}

}