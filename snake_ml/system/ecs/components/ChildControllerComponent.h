#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class ChildControllerComponent : public IComponentCastableImpl<ChildControllerComponent>
{
public:
	virtual ~ChildControllerComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::ChildControllerComponent; }

	uint32_t m_parentId;
	vector m_parentPosition, m_parentPreviousPosition;
	vector m_parentRotation, m_parentPreviousRotation;
};

class ChildControllerComponentConstructionVisitor : public ConstructionVisitor
{
public:
	ChildControllerComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::ChildControllerComponent; }
	void Visit(Iterator* it, Entity& entity);
};

REGISTER_COMPONENT(ChildControllerComponent)

}