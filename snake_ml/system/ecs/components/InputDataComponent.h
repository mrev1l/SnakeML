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

class InputDataComponentConstructionVisitor : public ConstructionVisitor
{
public:
	InputDataComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::InputDataComponent; }
	void Visit(Iterator* it, Entity& entity) override;
};

REGISTER_COMPONENT(InputDataComponent);

}