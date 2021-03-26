#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{
namespace system
{

class CameraComponent : public IComponent
{
public:
	virtual ~CameraComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::CameraComponent; }

	math::vector m_eyePosition;
	math::vector m_focusPoint;
	math::vector m_upDirection;
};
REGISTER_TYPE(CameraComponent);

}
}