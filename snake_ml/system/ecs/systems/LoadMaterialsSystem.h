// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "third_party/rapidjson/document.h" // rapidjson's DOM-style API

#include "system/ecs/components/MaterialComponent.h"
#include "system/ecs/ISystem.h"

namespace snakeml
{
namespace system
{

class LoadMaterialsSystem : public ISystem
{
public:
	void Execute() override;

private:
	static void ParseJsonString(const char* jsonBuffer, rapidjson::Document& outJson);
	static void ParseEntityId(const rapidjson::Document& json, uint32_t& outId);
	static void ParseVerticesArray(const rapidjson::Document& json, std::vector<std::pair<types::vec3<float>, types::vec2<float>>>& outVertices,
		std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries);
	static void ParseIndicesArray(const rapidjson::Document& json, std::vector<uint16_t>& indicesArray);
	static void ParseVSName(const rapidjson::Document& json, std::wstring& outVSName);
	static void ParsePSName(const rapidjson::Document& json, std::wstring& outPSName);
	static void ParseTexturePath(const rapidjson::Document& json, std::wstring& outTexturePath);
};

}
}