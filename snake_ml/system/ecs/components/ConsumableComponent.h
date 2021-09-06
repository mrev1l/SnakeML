#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class ConsumableComponent : public IComponentCastableImpl<ConsumableComponent>
{
public:
	virtual ~ConsumableComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::ConsumableComponent; }
};
REGISTER_TYPE(ConsumableComponent);

}
