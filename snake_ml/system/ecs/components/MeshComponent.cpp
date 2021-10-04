// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "MeshComponent.h"

#include "utils/rapidjson_utils.h"

namespace snakeml
{

void MeshComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	MeshComponentIterator& container = *it->As<MeshComponentIterator>();
	MeshComponent& mesh = container.Add();

	const auto vertexJsonCallback = [&mesh](const rapidjson::Value* element) -> void
	{
		std::pair<float3, float2> vertex;
		if (element->HasMember(k_posValueName))
		{
			RapidjsonUtils::ParseFloat3Value(*element, k_posValueName, vertex.first);
		}
		if (element->HasMember(k_uvValueName))
		{
			RapidjsonUtils::ParseFloat2Value(*element, k_uvValueName, vertex.second);
		}
		mesh.m_vertices.push_back(vertex);
	};

	RapidjsonUtils::ParseArrayValue(m_description, k_verticesValueName, vertexJsonCallback);

	AttachComponentToEntity(mesh, entity);
}

}