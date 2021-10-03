#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class MeshComponent : public IComponentCastableImpl<MeshComponent>
{
public:
	virtual ~MeshComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::MeshComponent; }

	std::vector<std::pair<float3, float2>> m_vertices;
	std::vector<uint16_t> m_indices;
};

class MeshComponentConstructionVisitor : public ConstructionVisitor
{
public:
	MeshComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() { return ComponentType::MeshComponent; }

	void Visit(Iterator* it, Entity& entity) override;

private:
	static void ParseMeshes_VerticesArray(const rapidjson::Value& json, std::vector<std::pair<float3, float2>>& outVertices);
};

REGISTER_COMPONENT(MeshComponent);

}
