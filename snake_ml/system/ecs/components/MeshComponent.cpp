// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "MeshComponent.h"

namespace snakeml
{

void MeshComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	MeshComponentIterator& container = *it->As<MeshComponentIterator>();

	MeshComponent& mesh = container.Add();

	ParseMeshes_VerticesArray(m_description, mesh.m_vertices);

	mesh.m_entityId = entity.m_entityId;
	entity.m_components.insert({ ComponentType::MeshComponent, &mesh });
}

void MeshComponentConstructionVisitor::ParseMeshes_VerticesArray(const rapidjson::Value& json, std::vector<std::pair<float3, float2>>& outVertices)
{
	if (!(json.HasMember("vertices") && json["vertices"].IsArray()))
	{
		return;
	}

	const rapidjson::GenericArray<true, rapidjson::Value>& verticesArray = json["vertices"].GetArray();
	rapidjson::Value::ConstValueIterator vertexIt = verticesArray.Begin();

	outVertices.resize(static_cast<std::vector<std::pair<float3, float3>>::size_type>(verticesArray.Size()));
	for (auto& vertex : outVertices)
	{
		if (vertexIt->HasMember("pos") && (*vertexIt)["pos"].IsArray() && (*vertexIt)["pos"].Size() == 3u)
		{
			const rapidjson::GenericArray<true, rapidjson::Value>& pos = (*vertexIt)["pos"].GetArray();
			vertex.first = { pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat() };
		}

		if (vertexIt->HasMember("uv") && (*vertexIt)["uv"].IsArray() && (*vertexIt)["uv"].Size() == 2u)
		{
			const rapidjson::GenericArray<true, rapidjson::Value>& uv = (*vertexIt)["uv"].GetArray();
			vertex.second = { uv[0].GetFloat(), uv[1].GetFloat() };
		}

		++vertexIt;
	}
}

}