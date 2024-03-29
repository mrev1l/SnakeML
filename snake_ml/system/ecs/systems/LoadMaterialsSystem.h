// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "third_party/rapidjson/document.h" // rapidjson's DOM-style API

#include "system/ecs/components/MaterialComponent.h"
#include "system/ecs/ISystem.h"

namespace snakeml
{
// todo rework into load level system
class LoadMaterialsSystem : public ISystem
{
public:
	LoadMaterialsSystem() : ISystem() {}

	void Execute() override;

private:
	static void ParseJsonString(const char* jsonBuffer, rapidjson::Document& outJson);
	
	static void ParseComponent_EntityId(const rapidjson::Value& json, uint32_t& outId);

	// ******* Parsing Material components *******
	static void ParseMaterials(const rapidjson::Document& json);
	static void ParseMaterialComponent_VerticesArray(const rapidjson::Value& json, std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries);
	static void ParseMaterialComponent_VSPath(const rapidjson::Value& json, std::wstring& outVSName);
	static void ParseMaterialComponent_PSPath(const rapidjson::Value& json, std::wstring& outPSName);
	static void ParseMaterialComponent_TexturePath(const rapidjson::Value& json, std::wstring& outTexturePath);
	// *******  *******  *******  *******  *******

	// ******* Parsing Transform components *******
	static void ParseTransforms(const rapidjson::Document& json);
	static void ParseTransformComponent_Position(const rapidjson::Value& json, vector& _outPosition);
	static void ParseTransformComponent_Rotation(const rapidjson::Value& json, vector& _outRotation);
	static void ParseTransformComponent_Scale(const rapidjson::Value& json, vector& _outScale);
	// *******  *******  *******  *******  ********

	// ******* Parsing Physics components *******
	static void ParsePhysicsComponents(const rapidjson::Document& json);
	static void ParsePhysicsComponents_ShapeDimensions(const rapidjson::Value& json, vector& _outShapeDimensions);
	static void ParsePhysicsComponents_ShapeMass(const rapidjson::Value& json, float& _outMass);
	static void ParsePhysicsComponents_IsDynamic(const rapidjson::Value& json, bool& _outIsDynamic);
	// *******  *******  *******  *******  ******

	// ******* Parsing Mesh components *******
	static void ParseMeshes(const rapidjson::Document& json);
	static void ParseMeshes_VerticesArray(const rapidjson::Value& json, std::vector<std::pair<float3, float2>>& outVertices);
	// ******* ******* ******* ******* *******

	static void ParseIndicesArray(const rapidjson::Document& json, std::vector<uint16_t>& indicesArray);
};

}
