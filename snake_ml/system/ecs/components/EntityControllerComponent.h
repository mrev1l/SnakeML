#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class EntityControllerComponent : public IComponentCastableImpl<EntityControllerComponent>
{
public:
	virtual ~EntityControllerComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::EntityControllerComponent; }

	vector m_inputVector = vector::zero;
};

class EntityControllerComponentConstructionVisitor : public ConstructionVisitor
{
public:
	EntityControllerComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::EntityControllerComponent; }
	void Visit(Iterator* it, Entity& entity) override;
};

REGISTER_COMPONENT(EntityControllerComponent);

}