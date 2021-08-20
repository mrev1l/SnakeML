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
REGISTER_TYPE(DebugRenderComponent);

}