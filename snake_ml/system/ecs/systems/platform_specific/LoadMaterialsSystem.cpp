// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "LoadMaterialsSystem.h"

#include "system/ecs/ECSManager.h"
#include "utils/win_utils.h"

namespace snakeml
{
namespace system
{

void LoadMaterialsSystem::Execute()
{
	constexpr uint32_t materialsNum = 1u;
	constexpr const char* jsonName = "D:\\Projects_Library\\snakeml_repo\\2dmaterialcomponentjson.txt"; // fix full path

	rapidjson::Document jsonDocument;
	std::string jsonString;
	WinUtils::LoadFileIntoBuffer(jsonName, jsonString);

	ParseJsonString(jsonString.c_str(), jsonDocument);

	DX12MaterialComponentIterator* it = (DX12MaterialComponentIterator*)IComponent::CreateIterator(ComponentType::DX12MaterialComponent, materialsNum);
	DX12MaterialComponent* material = (DX12MaterialComponent*)it->At(0);

	ParseEntityId(jsonDocument, material->m_entityId);
	ParseVerticesArray(jsonDocument, material->m_vertices);
	ParseIndicesArray(jsonDocument, material->m_indices);
	ParseVSName(jsonDocument, material->m_vs);
	ParsePSName(jsonDocument, material->m_ps);
	ParseVertexInputLayout(jsonDocument, material->m_vsInputLayout);
	ParseVertexInputParamLayout(jsonDocument, material->m_vsParamLayout);

	ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::DX12MaterialComponent, it);
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

void LoadMaterialsSystem::ParseVerticesArray(const rapidjson::Document& json, std::vector<std::pair<math::vec3<float>, math::vec3<float>>>& outVertices)
{
	ASSERT(json.HasMember("vertices") && json["vertices"].IsArray(), "Invalid vertices json");

	const rapidjson::GenericArray<true, rapidjson::Value>& verticesArray = json["vertices"].GetArray();
	rapidjson::Value::ConstValueIterator vertexIt = verticesArray.Begin();

	outVertices.resize(static_cast<std::vector<std::pair<math::vec3<float>, math::vec3<float>>>::size_type>(verticesArray.Size()));
	for (auto& vertex : outVertices)
	{
		ASSERT(vertexIt->HasMember("pos") && (*vertexIt)["pos"].IsArray() && (*vertexIt)["pos"].Size() == 3u, "Invalid vertices json");
		ASSERT(vertexIt->HasMember("color") && (*vertexIt)["color"].IsArray() && (*vertexIt)["color"].Size() == 3u, "Invalid vertices json");

		const rapidjson::GenericArray<true, rapidjson::Value>& pos = (*vertexIt)["pos"].GetArray();
		const rapidjson::GenericArray<true, rapidjson::Value>& color = (*vertexIt)["color"].GetArray();

		vertex.first = { pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat() };
		vertex.second = { color[0].GetFloat(), color[1].GetFloat(), color[2].GetFloat() };

		++vertexIt;
	}
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

	std::string ps = json["vs"].GetString();
	wchar_t* wPSName = nullptr;
	snakeml::WinUtils::StringToWstring(ps.c_str(), wPSName);
	outVSName = std::wstring(wPSName); // TODO: we don't actually copy it in ctor do we?
	delete wPSName;
}

void LoadMaterialsSystem::ParsePSName(const rapidjson::Document& json, std::wstring& outPSName)
{
	ASSERT(json.HasMember("ps") && json["ps"].IsString(), "Invalid ps json");

	std::string ps = json["ps"].GetString();
	wchar_t* wPSName = nullptr;
	snakeml::WinUtils::StringToWstring(ps.c_str(), wPSName);
	outPSName = std::wstring(wPSName); // TODO: we don't actually copy it in ctor do we?
	delete wPSName;
}

void LoadMaterialsSystem::ParseVertexInputLayout(const rapidjson::Document& json, std::vector<D3D12_INPUT_ELEMENT_DESC>& outLayout)
{
	ASSERT(json.HasMember("vertex_input_layout") && json["vertex_input_layout"].IsArray(), "Invalid vertex_input_layout json");
	const rapidjson::GenericArray<true, rapidjson::Value>& vsInputJson = json["vertex_input_layout"].GetArray();

	outLayout.resize(static_cast<std::vector<D3D12_INPUT_ELEMENT_DESC>::size_type>(vsInputJson.Size()));
	rapidjson::Value::ConstValueIterator vsInputLayoutIt = vsInputJson.Begin();

	for (auto& inputElement : outLayout)
	{
		ASSERT(vsInputLayoutIt->HasMember("semanticName") && (*vsInputLayoutIt)["semanticName"].IsString(), "Invalid vertex_input_layout json");
		ASSERT(vsInputLayoutIt->HasMember("semanticIdx") && (*vsInputLayoutIt)["semanticIdx"].IsUint(), "Invalid vertex_input_layout json");
		ASSERT(vsInputLayoutIt->HasMember("format") && (*vsInputLayoutIt)["format"].IsInt(), "Invalid vertex_input_layout json");
		ASSERT(vsInputLayoutIt->HasMember("inputSlot") && (*vsInputLayoutIt)["inputSlot"].IsUint(), "Invalid vertex_input_layout json");
		ASSERT(vsInputLayoutIt->HasMember("alignedByteOffset") && (*vsInputLayoutIt)["alignedByteOffset"].IsUint(), "Invalid vertex_input_layout json");
		ASSERT(vsInputLayoutIt->HasMember("inputSlotClass") && (*vsInputLayoutIt)["inputSlotClass"].IsInt(), "Invalid vertex_input_layout json");
		ASSERT(vsInputLayoutIt->HasMember("instanceDataStepRate") && (*vsInputLayoutIt)["instanceDataStepRate"].IsUint(), "Invalid vertex_input_layout json");

		const std::string name = (*vsInputLayoutIt)["semanticName"].GetString();
		inputElement.SemanticName = new char[name.size() + 1]; // TODO: make sure to delete
		strcpy_s((char*)(inputElement.SemanticName), name.size() + 1, name.c_str());

		inputElement.SemanticIndex = (*vsInputLayoutIt)["semanticIdx"].GetUint();
		inputElement.Format = static_cast<DXGI_FORMAT>((*vsInputLayoutIt)["format"].GetInt());
		inputElement.InputSlot = (*vsInputLayoutIt)["inputSlot"].GetUint();
		inputElement.AlignedByteOffset = (*vsInputLayoutIt)["alignedByteOffset"].GetUint();
		inputElement.InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>((*vsInputLayoutIt)["inputSlotClass"].GetInt());
		inputElement.InstanceDataStepRate = (*vsInputLayoutIt)["instanceDataStepRate"].GetUint();

		++vsInputLayoutIt;
	}
}

void LoadMaterialsSystem::ParseVertexInputParamLayout(const rapidjson::Document& json, DX12MaterialComponent::VSParamLayout& outLayout)
{
	ASSERT(json.HasMember("vertex_input_param_layout") && json["vertex_input_param_layout"].IsObject(), "Invalid vertex_input_param_layout json");

	const rapidjson::Value& vsInputParamLayoutJson = json["vertex_input_param_layout"];
	ASSERT(vsInputParamLayoutJson.HasMember("num32BitValues") && vsInputParamLayoutJson["num32BitValues"].IsUint(), "Invalid vertex_input_param_layout json");
	ASSERT(vsInputParamLayoutJson.HasMember("shaderRegister") && vsInputParamLayoutJson["shaderRegister"].IsUint(), "Invalid vertex_input_param_layout json");
	ASSERT(vsInputParamLayoutJson.HasMember("registerSpace") && vsInputParamLayoutJson["registerSpace"].IsUint(), "Invalid vertex_input_param_layout json");
	ASSERT(vsInputParamLayoutJson.HasMember("visibility") && vsInputParamLayoutJson["visibility"].IsInt(), "Invalid vertex_input_param_layout json");

	outLayout.num32BitValues = vsInputParamLayoutJson["num32BitValues"].GetUint();
	outLayout.shaderRegister = vsInputParamLayoutJson["shaderRegister"].GetUint();
	outLayout.registerSpace = vsInputParamLayoutJson["registerSpace"].GetUint();
	outLayout.visibility = static_cast<D3D12_SHADER_VISIBILITY>(vsInputParamLayoutJson["visibility"].GetInt());
}

}
}