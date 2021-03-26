// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{
namespace system
{

class TransformComponent : public IComponent
{
public:
	virtual ~TransformComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::TransformComponent; }

	math::vector m_position;
	math::vector m_rotation;
	math::vector m_scale;
};
REGISTER_TYPE(TransformComponent);

}
}