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
	
	static void ParseComponent_EntityId(const rapidjson::Value& json, uint32_t& outId);

	// ******* Parsing Material components *******
	static void ParseMaterials(const rapidjson::Document& json);
	static void ParseMaterialComponent_VerticesArray(const rapidjson::Value& json, std::vector<std::pair<types::vec3<float>, types::vec2<float>>>& outVertices,
		std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries);
	static void ParseMaterialComponent_VSPath(const rapidjson::Value& json, std::wstring& outVSName);
	static void ParseMaterialComponent_PSPath(const rapidjson::Value& json, std::wstring& outPSName);
	static void ParseMaterialComponent_TexturePath(const rapidjson::Value& json, std::wstring& outTexturePath);
	// *******  *******  *******  *******  *******

	// ******* Parsing Transform components *******
	static void ParseTransforms(const rapidjson::Document& json);
	static void ParseTransformComponent_Position(const rapidjson::Value& json, math::vector& _outPosition);
	static void ParseTransformComponent_Rotation(const rapidjson::Value& json, math::vector& _outRotation);
	static void ParseTransformComponent_Scale(const rapidjson::Value& json, math::vector& _outScale);
	// *******  *******  *******  *******  ********

	// ******* Parsing Physics components *******
	static void ParsePhysicsComponents(const rapidjson::Document& json);
	static void ParsePhysicsComponents_ShapeDimensions(const rapidjson::Value& json, math::vector& _outShapeDimensions);
	static void ParsePhysicsComponents_ShapeMass(const rapidjson::Value& json, float& _outMass);
	// *******  *******  *******  *******  ******
	static void ParseIndicesArray(const rapidjson::Document& json, std::vector<uint16_t>& indicesArray);

};

}
}