#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class CameraComponent : public IComponent
{
public:
	virtual ~CameraComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::CameraComponent; }

	vector m_eyePosition;
	vector m_focusPoint;
	vector m_upDirection;
};
REGISTER_TYPE(CameraComponent);

}
