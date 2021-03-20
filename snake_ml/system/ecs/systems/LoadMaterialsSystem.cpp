// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "LoadMaterialsSystem.h"

#include "system/drivers/win/os/helpers/win_utils.h"

#include "system/ecs/ECSManager.h"

namespace snakeml
{
namespace system
{

void LoadMaterialsSystem::Execute()
{
	constexpr uint32_t materialsNum = 1u;
	constexpr const char* jsonName = "data\\assets\\levels\\test\\2dmaterialcomponentjson.txt";

	rapidjson::Document jsonDocument;
	std::string jsonString;
	WinUtils::LoadFileIntoBuffer(jsonName, jsonString);

	ParseJsonString(jsonString.c_str(), jsonDocument);

	MaterialComponentIterator* it = (MaterialComponentIterator*)IComponent::CreateIterator(ComponentType::MaterialComponent, materialsNum);
	MaterialComponent& material = *(MaterialComponent*)it->At(0);

	ParseEntityId(jsonDocument, material.m_entityId);
	ParseVerticesArray(jsonDocument, material.m_vertices, material.m_inputLayoutEntries);
	ParseVSName(jsonDocument, material.m_vs);
	ParsePSName(jsonDocument, material.m_ps);
	ParseTexturePath(jsonDocument, material.m_texturePath);

	ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::MaterialComponent, it);
}

void LoadMaterialsSystem::ParseJsonString(const char* jsonBuffer, rapidjson::Document& outJson)
{
	/// jsonBuffer needs to have the same scope as outJson to utilize ParseInsitu
	ASSERT(!outJson.ParseInsitu(const_cast<char*>(jsonBuffer)).HasParseError(), "Failed to parse JSON"); // remove const cast => error C2664: 'void rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>::Free(void *)': cannot convert argument 1 from 'const char *' to 'void *'
	ASSERT(outJson.IsObject(), "Invalid JSON");
}

void LoadMaterialsSystem::ParseEntityId(const rapidjson::Document& json, uint32_t& outId)
{
	ASSERT(json.HasMember("entityId") && json["entityId"].IsUint(), "Invalid entityId json");
	outId = json["entityId"].GetUint();
}

void LoadMaterialsSystem::ParseVerticesArray(const rapidjson::Document& json, std::vector<std::pair<types::vec3<float>, types::vec2<float>>>& outVertices,
	std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries)
{
	ASSERT(json.HasMember("vertices") && json["vertices"].IsArray(), "Invalid vertices json");

	const rapidjson::GenericArray<true, rapidjson::Value>& verticesArray = json["vertices"].GetArray();
	rapidjson::Value::ConstValueIterator vertexIt = verticesArray.Begin();

	outVertices.resize(static_cast<std::vector<std::pair<types::vec3<float>, types::vec3<float>>>::size_type>(verticesArray.Size()));
	for (auto& vertex : outVertices)
	{
		ASSERT(vertexIt->HasMember("pos") && (*vertexIt)["pos"].IsArray() && (*vertexIt)["pos"].Size() == 3u, "Invalid vertices json");
		//ASSERT(vertexIt->HasMember("color") && (*vertexIt)["color"].IsArray() && (*vertexIt)["color"].Size() == 3u, "Invalid vertices json");
		ASSERT(vertexIt->HasMember("uv") && (*vertexIt)["uv"].IsArray() && (*vertexIt)["uv"].Size() == 2u, "Invalid vertices json");

		const rapidjson::GenericArray<true, rapidjson::Value>& pos = (*vertexIt)["pos"].GetArray();
		//const rapidjson::GenericArray<true, rapidjson::Value>& color = (*vertexIt)["color"].GetArray();
		const rapidjson::GenericArray<true, rapidjson::Value>& uv = (*vertexIt)["uv"].GetArray();

		vertex.first = { pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat() };
		//vertex.second = { color[0].GetFloat(), color[1].GetFloat(), color[2].GetFloat() };
		vertex.second = { uv[0].GetFloat(), uv[1].GetFloat()};

		++vertexIt;
	}

	inputLayoutEntries.push_back(MaterialComponent::InputLayoutEntries::Position);
	inputLayoutEntries.push_back(MaterialComponent::InputLayoutEntries::UV);
}

void LoadMaterialsSystem::ParseIndicesArray(const rapidjson::Document& json, std::vector<uint16_t>& indicesArray)
{
	ASSERT(json.HasMember("indices") && json["indices"].IsArray(), "Invalid indices json");
	const rapidjson::GenericArray<true, rapidjson::Value>& indicesJson = json["indices"].GetArray();

	indicesArray.resize(static_cast<std::vector<uint16_t>::size_type>(indicesJson.Size()));
	rapidjson::Value::ConstValueIterator indicesIt = indicesJson.Begin();

	for (uint16_t& index : indicesArray)
	{
		index = (indicesIt++)->GetInt();
	}
}

void LoadMaterialsSystem::ParseVSName(const rapidjson::Document& json, std::wstring& outVSName)
{
	ASSERT(json.HasMember("vs") && json["vs"].IsString(), "Invalid vs json");

	std::string vs = json["vs"].GetString();
	WinUtils::StringToWstring(vs.c_str(), outVSName);
}

void LoadMaterialsSystem::ParsePSName(const rapidjson::Document& json, std::wstring& outPSName)
{
	ASSERT(json.HasMember("ps") && json["ps"].IsString(), "Invalid ps json");

	std::string ps = json["ps"].GetString();
	WinUtils::StringToWstring(ps.c_str(), outPSName);
}

void LoadMaterialsSystem::ParseTexturePath(const rapidjson::Document& json, std::wstring& outTexturePath)
{
	const bool hasTexturePath = json.HasMember("texture");
	const bool isString = json["texture"].IsString();
	if (hasTexturePath && isString)
	{
		WinUtils::StringToWstring(json["texture"].GetString(), outTexturePath);
	}
}


}
}