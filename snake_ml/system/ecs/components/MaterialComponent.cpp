// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "MaterialComponent.h"

#include "utils/rapidjson_utils.h"

#include "system/drivers/win/os/helpers/win_utils.h"// TODO Fix closs platform

namespace snakeml
{

void MaterialComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	MaterialComponentIterator& container = *it->As<MaterialComponentIterator>();
	MaterialComponent& material = container.Add();

	InitInputLayoutDescription(m_description, material.m_inputLayoutEntries);
	RapidjsonUtils::ParseWstringValue(m_description, k_vsValueName, material.m_vs);
	RapidjsonUtils::ParseWstringValue(m_description, k_psValueName, material.m_ps);
	//RapidjsonUtils::ParseWstringValue(m_description, k_textureValueName, material.m_texturePath);

	const auto ParseElement = [this, &material](const rapidjson::Value* elementIt) -> void
	{
		std::wstring texture;
		// TODO : Abstract
		const std::string string = elementIt->GetString();
		win::WinUtils::StringToWstring(string.c_str(), texture); // TODO Fix closs platform
		material.m_textures.push_back(texture);
	};

	RapidjsonUtils::ParseArrayValue(m_description, k_texturesValueName, false, ParseElement);

	AttachComponentToEntity(material, entity);
}

void MaterialComponentConstructionVisitor::InitInputLayoutDescription(const rapidjson::Value& json, std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries)
{
	// TODO: read from json ? shader reflection ?

	inputLayoutEntries.push_back(MaterialComponent::InputLayoutEntries::Position);
	inputLayoutEntries.push_back(MaterialComponent::InputLayoutEntries::UV);
}

}