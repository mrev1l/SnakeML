// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "LoadMaterialsSystem.h"

#include "system/drivers/win/os/helpers/win_utils.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/components/MeshComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/ConsumableComponent.h"

namespace snakeml
{

void LoadMaterialsSystem::Execute()
{
	constexpr const char* jsonName = "data\\assets\\levels\\level_0\\level_0.txt";
	rapidjson::Document jsonDocument;
	std::string jsonString;
	win::WinUtils::LoadFileIntoBuffer(jsonName, jsonString);

	ParseJsonString(jsonString.c_str(), jsonDocument);

	ParseMaterials(jsonDocument);
	ParseMeshes(jsonDocument);
	ParseTransforms(jsonDocument);
	ParsePhysicsComponents(jsonDocument);
	ParseConsumables(jsonDocument);
}

void LoadMaterialsSystem::ParseJsonString(const char* jsonBuffer, rapidjson::Document& outJson)
{
	/// jsonBuffer needs to have the same scope as outJson to utilize ParseInsitu
	ASSERT(!outJson.ParseInsitu(const_cast<char*>(jsonBuffer)).HasParseError(), "Failed to parse JSON"); // remove const cast => error C2664: 'void rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>::Free(void *)': cannot convert argument 1 from 'const char *' to 'void *'
	ASSERT(outJson.IsObject(), "Invalid JSON");
}

void LoadMaterialsSystem::ParseComponent_EntityId(const rapidjson::Value& json, uint32_t& outId)
{
	ASSERT(json.HasMember("entityId") && json["entityId"].IsUint(), "Invalid entityId json");
	outId = json["entityId"].GetUint();
}

void LoadMaterialsSystem::ParseMaterials(const rapidjson::Document& json)
{
	const bool hasMaterials = json.HasMember("material_components") && json["material_components"].IsArray();
	if (hasMaterials)
	{
		const rapidjson::GenericArray<true, rapidjson::Value>& materialsArray = json["material_components"].GetArray();
		const size_t materialsCount = materialsArray.Size();
		MaterialComponentIterator* it = (MaterialComponentIterator*)IComponent::CreateIterator(ComponentType::MaterialComponent, materialsCount);
		for (size_t i = 0u; i < materialsCount; ++i)
		{
			MaterialComponent& material = it->At(i);
			const rapidjson::Value& materialJson = *(materialsArray.Begin() + i);

			ParseComponent_EntityId(materialJson, material.m_entityId);
			ParseMaterialComponent_VerticesArray(materialJson, material.m_inputLayoutEntries);
			ParseMaterialComponent_VSPath(materialJson, material.m_vs);
			ParseMaterialComponent_PSPath(materialJson, material.m_ps);
			ParseMaterialComponent_TexturePath(materialJson, material.m_texturePath);
		}

		ECSManager::GetInstance()->InsertComponents<MaterialComponentIterator>(it);
	}
}

void LoadMaterialsSystem::ParseMaterialComponent_VerticesArray(const rapidjson::Value& json, std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries)
{
	// TODO: read from json ? shader reflection ?

	inputLayoutEntries.push_back(MaterialComponent::InputLayoutEntries::Position);
	inputLayoutEntries.push_back(MaterialComponent::InputLayoutEntries::UV);
}

void LoadMaterialsSystem::ParseMaterialComponent_VSPath(const rapidjson::Value& json, std::wstring& outVSName)
{
	//ASSERT(json.HasMember("vs") && json["vs"].IsString(), "Invalid vs json");
	if (!(json.HasMember("vs") && json["vs"].IsString()))
	{
		return;
	}

	std::string vs = json["vs"].GetString();
	win::WinUtils::StringToWstring(vs.c_str(), outVSName);
}

void LoadMaterialsSystem::ParseMaterialComponent_PSPath(const rapidjson::Value& json, std::wstring& outPSName)
{
	//ASSERT(json.HasMember("ps") && json["ps"].IsString(), "Invalid ps json");
	if (!(json.HasMember("ps") && json["ps"].IsString()))
	{
		return;
	}

	std::string ps = json["ps"].GetString();
	win::WinUtils::StringToWstring(ps.c_str(), outPSName);
}

void LoadMaterialsSystem::ParseMaterialComponent_TexturePath(const rapidjson::Value& json, std::wstring& outTexturePath)
{
	const bool hasTexturePath = json.HasMember("texture");
	const bool isString = hasTexturePath ? json["texture"].IsString() : false;
	if (hasTexturePath && isString)
	{
		win::WinUtils::StringToWstring(json["texture"].GetString(), outTexturePath);
	}
}

void LoadMaterialsSystem::ParseTransforms(const rapidjson::Document& json)
{
	const bool hasTransforms = json.HasMember("transform_components") && json["transform_components"].IsArray();
	if (hasTransforms)
	{
		const rapidjson::GenericArray<true, rapidjson::Value>& transformsArray = json["transform_components"].GetArray();
		const size_t transformsCount = transformsArray.Size();
		TransformComponentIterator* it = (TransformComponentIterator*)IComponent::CreateIterator(ComponentType::TransformComponent, transformsCount);
		for (size_t i = 0u; i < transformsCount; ++i)
		{
			TransformComponent& transform = it->At(i);
			const rapidjson::Value& transformJson = *(transformsArray.Begin() + i);

			ParseComponent_EntityId(transformJson, transform.m_entityId);
			ParseTransformComponent_Position(transformJson, transform.m_position);
			ParseTransformComponent_Rotation(transformJson, transform.m_rotation);
			ParseTransformComponent_Scale(transformJson, transform.m_scale);
		}

		ECSManager::GetInstance()->InsertComponents<TransformComponentIterator>(it);
	}
}

void LoadMaterialsSystem::ParseTransformComponent_Position(const rapidjson::Value& json, vector& _outPosition)
{
	ASSERT(json.HasMember("position") && json["position"].IsArray() && json["position"].Size() == 3u, "Invalid position json");

	const rapidjson::GenericArray<true, rapidjson::Value>& positionJson = json["position"].GetArray();

	_outPosition = { positionJson[0].GetFloat(), positionJson[1].GetFloat(), positionJson[2].GetFloat() };
}

void LoadMaterialsSystem::ParseTransformComponent_Rotation(const rapidjson::Value& json, vector& _outRotation)
{
	ASSERT(json.HasMember("rotation") && json["rotation"].IsArray() && json["rotation"].Size() == 3u, "Invalid rotation json");

	const rapidjson::GenericArray<true, rapidjson::Value>& rotationJson = json["rotation"].GetArray();

	_outRotation = { rotationJson[0].GetFloat(), rotationJson[1].GetFloat(), rotationJson[2].GetFloat() };
}

void LoadMaterialsSystem::ParseTransformComponent_Scale(const rapidjson::Value& json, vector& _outScale)
{
	ASSERT(json.HasMember("scale") && json["scale"].IsArray() && json["scale"].Size() == 3u, "Invalid scale json");

	const rapidjson::GenericArray<true, rapidjson::Value>& scaleJson = json["scale"].GetArray();

	_outScale = { scaleJson[0].GetFloat(), scaleJson[1].GetFloat(), scaleJson[2].GetFloat() };
}

void LoadMaterialsSystem::ParsePhysicsComponents(const rapidjson::Document& json)
{
	const bool hasPhysicsComponents = json.HasMember("physics_components") && json["physics_components"].IsArray();
	if (hasPhysicsComponents)
	{
		const rapidjson::GenericArray<true, rapidjson::Value>& physicsComponentsArray = json["physics_components"].GetArray();
		const size_t physicsComponentsCount = physicsComponentsArray.Size();
		PhysicsComponentIterator* it = (PhysicsComponentIterator*)IComponent::CreateIterator(ComponentType::PhysicsComponent, physicsComponentsCount);
		for (size_t i = 0u; i < physicsComponentsCount; ++i)
		{
			PhysicsComponent& physicsComponent = it->At(i);
			const rapidjson::Value& physicsComponentJson = *(physicsComponentsArray.Begin() + i);

			ParseComponent_EntityId(physicsComponentJson, physicsComponent.m_entityId);
			ParsePhysicsComponents_ShapeDimensions(physicsComponentJson, physicsComponent.m_shape.m_dimensions);
			ParsePhysicsComponents_ShapeMass(physicsComponentJson, physicsComponent.m_shape.m_mass);
			ParsePhysicsComponents_IsDynamic(physicsComponentJson, physicsComponent.m_isDynamic);
			ParsePhysicsComponents_CollisionChannel(physicsComponentJson, physicsComponent.m_collisionChannel);
			ParsePhysicsComponents_CollisionFilter(physicsComponentJson, physicsComponent.m_collisionFilter);
		}

		ECSManager::GetInstance()->InsertComponents<PhysicsComponentIterator>(it);
	}
}

void LoadMaterialsSystem::ParsePhysicsComponents_ShapeDimensions(const rapidjson::Value& json, vector& _outShapeDimensions)
{
	ASSERT(json.HasMember("shape_dimensions") && json["shape_dimensions"].IsArray() && json["shape_dimensions"].Size() == 3u, "Invalid shape dimension json");

	const rapidjson::GenericArray<true, rapidjson::Value>& shapeDimensionsJson = json["shape_dimensions"].GetArray();

	_outShapeDimensions = { shapeDimensionsJson[0].GetFloat(), shapeDimensionsJson[1].GetFloat(), shapeDimensionsJson[2].GetFloat() };
}

void LoadMaterialsSystem::ParsePhysicsComponents_ShapeMass(const rapidjson::Value& json, float& _outMass)
{
	ASSERT(json.HasMember("shape_mass") && json["shape_mass"].IsFloat(), "Invalid shape mass json");

	_outMass = json["shape_mass"].GetFloat();
}

void LoadMaterialsSystem::ParsePhysicsComponents_IsDynamic(const rapidjson::Value& json, bool& _outIsDynamic)
{
	ASSERT(json.HasMember("is_dynamic") && json["is_dynamic"].IsBool(), "Invalid is dynamic json");

	_outIsDynamic = json["is_dynamic"].GetBool();
}

void LoadMaterialsSystem::ParsePhysicsComponents_CollisionChannel(const rapidjson::Value& json, CollisionChannel& _outCollisionChannel)
{
	ASSERT(json.HasMember("collision_channel") && json["collision_channel"].IsUint(), "Invalid collision channel json");

	const uint32_t collisionChannelData = json["collision_channel"].GetUint();
	_outCollisionChannel = static_cast<CollisionChannel>(collisionChannelData);
}

void LoadMaterialsSystem::ParsePhysicsComponents_CollisionFilter(const rapidjson::Value& json, CollisionChannel& _outCollisionFilter)
{
	ASSERT(json.HasMember("collision_filter") && json["collision_filter"].IsUint(), "Invalid collision filter json");

	const uint32_t collisionChannelData = json["collision_filter"].GetUint();
	_outCollisionFilter = static_cast<CollisionChannel>(collisionChannelData);
}

void LoadMaterialsSystem::ParseMeshes(const rapidjson::Document& json)
{
	const bool hasMeshes = json.HasMember("mesh_components") && json["mesh_components"].IsArray();
	if (hasMeshes)
	{
		const rapidjson::GenericArray<true, rapidjson::Value>& meshComponentsArray = json["mesh_components"].GetArray();
		const size_t meshComponentsCount = meshComponentsArray.Size();
		MeshComponentIterator* it = (MeshComponentIterator*)IComponent::CreateIterator(ComponentType::MeshComponent, meshComponentsCount);
		for (size_t i = 0u; i < meshComponentsCount; ++i)
		{
			MeshComponent& meshComponent = it->At(i);
			const rapidjson::Value& meshComponentJson = *(meshComponentsArray.Begin() + i);

			ParseComponent_EntityId(meshComponentJson, meshComponent.m_entityId);
			ParseMeshes_VerticesArray(meshComponentJson, meshComponent.m_vertices);
		}

		ECSManager::GetInstance()->InsertComponents<MeshComponentIterator>(it);
	}
}

void LoadMaterialsSystem::ParseMeshes_VerticesArray(const rapidjson::Value& json, std::vector<std::pair<float3, float2>>& outVertices)
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

void LoadMaterialsSystem::ParseConsumables(const rapidjson::Document& json)
{
	const bool hasConsumables = json.HasMember("consumable_components") && json["consumable_components"].IsArray();
	if (hasConsumables)
	{
		const rapidjson::GenericArray<true, rapidjson::Value>& consumablesDataArray = json["consumable_components"].GetArray();
		const size_t consumableComponentsCount = consumablesDataArray.Size();

		ConsumableComponentIterator* it = IComponent::CreateIterator(ComponentType::ConsumableComponent, consumableComponentsCount)->As<ConsumableComponentIterator>();
		for (size_t i = 0u; i < consumableComponentsCount; ++i)
		{
			ConsumableComponent& consumableComponent = it->At(i);
			const rapidjson::Value& consumableComponentJson = *(consumablesDataArray.Begin() + i);

			ParseComponent_EntityId(consumableComponentJson, consumableComponent.m_entityId);
		}

		ECSManager::GetInstance()->InsertComponents<ConsumableComponentIterator>(it);
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


}
