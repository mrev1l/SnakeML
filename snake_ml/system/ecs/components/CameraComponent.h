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

	types::vec4<float> m_eyePosition;
	types::vec4<float> m_focusPoint;
	types::vec4<float> m_upDirection;
};
REGISTER_TYPE(CameraComponent);

}
}