// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "LevelLoadingSystem.h"

#include "system/drivers/win/os/helpers/win_utils.h"

#include "system/ecs/ECSManager.h"

#include "utils/rapidjson_utils.h"

#ifdef GetObject
#undef GetObject
#endif

namespace snakeml
{

void LevelLoadingSystem::Execute()
{
	LoadDataFile(k_levelFile, m_levelFileBuffer, m_levelDocument);
	LoadDataFile(k_templatesFile, m_templatesFileBuffer, m_templatesDocument);

	LoadComponentsDescription(m_templatesDocument.GetObject(), m_componentsDesc);
	LoadTemplatesDescription(m_templatesDocument.GetObject(), m_templatesDesc);

	InitializeComponentsStorage();

	LoadEntities(m_levelDocument.GetObject(), m_componentsDesc, m_templatesDesc);
}

void LevelLoadingSystem::LoadDataFile(const char* fileName, std::string& _outFileBuffer, rapidjson::Document& _outFileDocument)
{
	win::WinUtils::LoadFileIntoBuffer(fileName, _outFileBuffer);
	/// FileBuffer needs to have the same scope as FileDocument to utilize ParseInsitu
	ASSERT(!_outFileDocument.ParseInsitu(const_cast<char*>(_outFileBuffer.c_str())).HasParseError(), "[LevelLoadingSystem::LoadDataFile] : Failed to parse JSON."); // TODO : remove const cast => error C2664: 'void rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>::Free(void *)': cannot convert argument 1 from 'const char *' to 'void *'
	ASSERT(_outFileDocument.IsObject(), "[LevelLoadingSystem::LoadDataFile] : Invalid JSON.");
}

void LevelLoadingSystem::LoadComponentsDescription(const rapidjson::Value& json, std::vector<ComponentDescription>& outComponentsDesc)
{
	const auto elementCallback = [&outComponentsDesc](const rapidjson::Value* elementIt) -> void
	{
		outComponentsDesc.push_back(LoadComponentDescription(*elementIt));
	};

	RapidjsonUtils::ParseArrayValue(json, k_componentsDescValueName, elementCallback);
}

LevelLoadingSystem::ComponentDescription LevelLoadingSystem::LoadComponentDescription(const rapidjson::Value& json)
{
	ASSERT(json.HasMember(k_descriptionValueName), "[LevelLoadingSystem::LoadComponentDescription] : Invalid template json");

	size_t componentId;
	ComponentType componentType;

	RapidjsonUtils::ParseUintValue(json, k_componentIdValueName, componentId);
	RapidjsonUtils::ParseEnumValue<ComponentType>(json, k_componentTypeIdValueName, componentType);

	return ComponentDescription { componentId, componentType, json[k_descriptionValueName] };
}

void LevelLoadingSystem::LoadTemplatesDescription(const rapidjson::Value& json, std::vector<Template>& outTemplates)
{
	const auto elementCallback = [&outTemplates](const rapidjson::Value* elementIt) -> void
	{
		outTemplates.push_back(LoadTemplateDescription(*elementIt));
	};

	RapidjsonUtils::ParseArrayValue(json, k_templatesValueName, elementCallback);
}

LevelLoadingSystem::Template LevelLoadingSystem::LoadTemplateDescription(const rapidjson::Value& json)
{
	size_t id;
	std::string name;
	std::vector<size_t> components;

	const auto elementCallback = [&components](const rapidjson::Value* elementIt) -> void
	{
		components.push_back(elementIt->GetUint());
	};

	RapidjsonUtils::ParseUintValue(json, k_templateIdValueName, id);
	RapidjsonUtils::ParseStringValue(json, k_nameValueName, name);
	RapidjsonUtils::ParseArrayValue(json, k_componentsValueName, elementCallback);

	return Template { id, name, components };
}

void LevelLoadingSystem::InitializeComponentsStorage()
{
	for (ComponentType type = static_cast<ComponentType>(0); type != ComponentType::Size; ++type)
	{
		IComponent::CreateIterator(type, Iterator::k_defaultCapacity);
	}
}

void LevelLoadingSystem::LoadEntities(const rapidjson::Value& json, const std::vector<ComponentDescription>& componentsDesc, const std::vector<Template>& templatesDesc)
{
	size_t entityIdCounter = 0;
	std::unordered_map<uint32_t, Entity>& entities = ECSManager::GetInstance()->GetEntities();
	const auto elementCallback = [&entities, &entityIdCounter, &componentsDesc, &templatesDesc](const rapidjson::Value* elementIt) -> void
	{
		Entity& entity = entities[entityIdCounter];
		entity.m_entityId = entityIdCounter;
		LoadEntity(*elementIt, componentsDesc, templatesDesc, entity);
		++entityIdCounter;
	};

	RapidjsonUtils::ParseArrayValue(json, k_levelEntitiesValueName, elementCallback);
}

void LevelLoadingSystem::LoadEntity(const rapidjson::Value& json, const std::vector<ComponentDescription>& componentsDesc, const std::vector<Template>& templatesDesc, Entity& outEntity)
{
	ASSERT(json.HasMember(k_transformValueName) && json[k_transformValueName].IsObject(), "[LevelLoadingSystem::LoadEntity] : Invalid entities json");

	size_t templateId;
	
	RapidjsonUtils::ParseStringValue(json, k_nameValueName, outEntity.m_name);
	RapidjsonUtils::ParseUintValue(json, k_templateIdValueName, templateId);

	const auto templateIt = std::find_if(templatesDesc.begin(), templatesDesc.end(), [templateId](const Template& a) { return a.id == templateId; });
	ASSERT(templateIt != templatesDesc.end(), "[LevelLoadingSystem::LoadEntity] : Invalid entities json");

	for (const auto componentId : templateIt->componentIds)
	{
		const auto componentIt = std::find_if(componentsDesc.begin(), componentsDesc.end(), [componentId](const ComponentDescription& a) { return a.id == componentId; });
		ASSERT(componentIt != componentsDesc.end(), "[LevelLoadingSystem::LoadEntity] : Invalid entities json");

		const ComponentType compType = static_cast<ComponentType>(componentIt->type);

		Iterator* container = ECSManager::GetInstance()->GetComponents(compType);
		const std::unique_ptr<ConstructionVisitor> cv = IComponent::CreateIteratorConstructionVisitor(compType, componentIt->json);
		container->Accept(cv, outEntity);
	}

	const rapidjson::Value& transformData = json[k_transformValueName];
	Iterator* transforms = ECSManager::GetInstance()->GetComponents(ComponentType::TransformComponent);
	const std::unique_ptr<ConstructionVisitor> cv = IComponent::CreateIteratorConstructionVisitor(ComponentType::TransformComponent, json[k_transformValueName]);
	transforms->Accept(cv, outEntity);
}

}
