#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{
namespace system
{

class MeshComponent : public IComponent
{
public:
	virtual ~MeshComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::MeshComponent; }

	std::vector<std::pair<types::vec3<float>, types::vec2<float>>> m_vertices;
	std::vector<uint16_t> m_indices;
};
REGISTER_TYPE(MeshComponent);

}
}
