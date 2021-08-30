#pragma once
#include "system/ecs/IComponent.h"
#include "system/input/InputManager.h"

namespace snakeml
{

class InputDataComponent : public IComponentCastableImpl<InputDataComponent>
{
public:
	virtual ~InputDataComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::InputDataComponent; }

	std::vector<InputManager::InputAxisData> m_axesInput;
};
REGISTER_TYPE(InputDataComponent);

}