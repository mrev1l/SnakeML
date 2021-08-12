#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class MeshComponent : public IComponent
{
public:
	virtual ~MeshComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::MeshComponent; }

	std::vector<std::pair<float3, float2>> m_vertices;
	std::vector<uint16_t> m_indices;
};
REGISTER_TYPE(MeshComponent);

}
