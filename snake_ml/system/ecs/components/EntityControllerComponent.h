#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class EntityControllerComponent : public IComponentCastableImpl<EntityControllerComponent>
{
public:
	virtual ~EntityControllerComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::EntityControllerComponent; }

	vector m_inputVector;
};
REGISTER_TYPE(EntityControllerComponent);

}