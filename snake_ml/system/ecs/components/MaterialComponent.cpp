// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "MaterialComponent.h"

#include "system/drivers/win/os/helpers/win_utils.h"// TODO Fix closs platform

namespace snakeml
{

void MaterialComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	MaterialComponentIterator& container = *it->As<MaterialComponentIterator>();

	MaterialComponent& material = container.Add();

	ParseMaterialComponent_VerticesArray(m_description, material.m_inputLayoutEntries);
	ParseMaterialComponent_VSPath(m_description, material.m_vs);
	ParseMaterialComponent_PSPath(m_description, material.m_ps);
	ParseMaterialComponent_TexturePath(m_description, material.m_texturePath);

	material.m_entityId = entity.m_entityId;
	entity.m_components.insert({ ComponentType::MaterialComponent, &material });
}

void MaterialComponentConstructionVisitor::ParseMaterialComponent_VerticesArray(const rapidjson::Value& json, std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries)
{
	// TODO: read from json ? shader reflection ?

	inputLayoutEntries.push_back(MaterialComponent::InputLayoutEntries::Position);
	inputLayoutEntries.push_back(MaterialComponent::InputLayoutEntries::UV);
}

void MaterialComponentConstructionVisitor::ParseMaterialComponent_VSPath(const rapidjson::Value& json, std::wstring& outVSName)
{
	//ASSERT(json.HasMember("vs") && json["vs"].IsString(), "Invalid vs json");
	if (!(json.HasMember("vs") && json["vs"].IsString()))
	{
		return;
	}

	std::string vs = json["vs"].GetString();
	win::WinUtils::StringToWstring(vs.c_str(), outVSName); // TODO Fix closs platform
}

void MaterialComponentConstructionVisitor::ParseMaterialComponent_PSPath(const rapidjson::Value& json, std::wstring& outPSName)
{
	//ASSERT(json.HasMember("ps") && json["ps"].IsString(), "Invalid ps json");
	if (!(json.HasMember("ps") && json["ps"].IsString()))
	{
		return;
	}

	std::string ps = json["ps"].GetString();
	win::WinUtils::StringToWstring(ps.c_str(), outPSName);// TODO Fix closs platform
}

void MaterialComponentConstructionVisitor::ParseMaterialComponent_TexturePath(const rapidjson::Value& json, std::wstring& outTexturePath)
{
	const bool hasTexturePath = json.HasMember("texture");
	const bool isString = hasTexturePath ? json["texture"].IsString() : false;
	if (hasTexturePath && isString)
	{
		win::WinUtils::StringToWstring(json["texture"].GetString(), outTexturePath);// TODO Fix closs platform
	}
}

}