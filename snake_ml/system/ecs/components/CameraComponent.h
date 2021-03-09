#pragma once

#include "system/ecs/IComponent.h"
#include "utils/snake_math.h"

namespace snakeml
{
namespace system
{

class CameraComponent : public IComponent
{
public:
	virtual ~CameraComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::CameraComponent; }

	math::vec4<float> m_eyePosition;
	math::vec4<float> m_focusPoint;
	math::vec4<float> m_upDirection;
};
REGISTER_TYPE(CameraComponent);

}
}