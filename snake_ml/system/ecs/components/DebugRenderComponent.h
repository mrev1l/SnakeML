#pragma once
#include "system/ecs/IComponent.h"
#include "utils/math/algorithms/AABB.h"

namespace snakeml
{

class DebugRenderComponent : public IComponentCastableImpl<DebugRenderComponent>
{
public:
	virtual ~DebugRenderComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::DebugRenderComponent; }

	bool m_isEnabled = false;
	AABB m_debugAABB;
};

class DebugRenderComponentConstructionVisitor : public ConstructionVisitor
{
public:
	DebugRenderComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() { return ComponentType::DebugRenderComponent; }
};

REGISTER_COMPONENT(DebugRenderComponent);

}