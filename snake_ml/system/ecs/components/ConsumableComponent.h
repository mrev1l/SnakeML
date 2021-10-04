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

class ConsumableComponentConstructionVisitor : public ConstructionVisitor
{
public:
	ConsumableComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::ConsumableComponent; }
	void Visit(Iterator* it, Entity& entity) override;
};

REGISTER_COMPONENT(ConsumableComponent);

}
