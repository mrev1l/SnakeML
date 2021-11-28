#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class ParentComponent : public IComponentCastableImpl<ParentComponent>
{
public:
	virtual ~ParentComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::ParentComponent; }

	uint32_t m_childId = -1;
	vector m_parentPreviousFramePosition;
	vector m_parentPreviousFrameRotation;
	vector m_parentPreviousMovementTickPosition;
	vector m_parentPreviousMovementTickRotation;
};

class ParentComponentConstructionVisitor : public ConstructionVisitor
{
public:
	ParentComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::ParentComponent; }
	void Visit(Iterator* it, Entity& entity);
};

REGISTER_COMPONENT(ParentComponent)

}