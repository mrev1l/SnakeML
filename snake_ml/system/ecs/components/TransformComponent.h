// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class TransformComponent : public ComponentBaseImpl<TransformComponent>
{
public:
	virtual ~TransformComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::TransformComponent; }

	vector m_position;
	vector m_rotation;
	vector m_scale;
};
REGISTER_TYPE(TransformComponent);

}
