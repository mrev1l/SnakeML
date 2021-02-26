// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"
#include "utils/snake_math.h"

namespace snakeml
{
namespace system
{

class TransformComponent : public IComponent
{
public:
	virtual ~TransformComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::TransformComponent; }

	math::vec4<float> m_position;
	math::vec4<float> m_rotation;
	math::vec4<float> m_scale;
};
REGISTER_TYPE(TransformComponent);

}
}