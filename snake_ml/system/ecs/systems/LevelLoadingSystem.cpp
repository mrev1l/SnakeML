// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "LevelLoadingSystem.h"

#include "system/drivers/win/os/helpers/win_utils.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/components/LevelRequestComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"

#include "utils/rapidjson_utils.h"

#ifdef GetObject
#undef GetObject
#endif

namespace snakeml
{

void LevelLoadingSystem::Update(float deltaTime)
{
	LevelRequestComponentIterator* levelRequestContainer = ECSManager::GetInstance()->GetComponents<LevelRequestComponentIterator>();
	if (levelRequestContainer)
	{
		ASSERT(levelRequestContainer->Size() <= 1, "[LevelLoadingSystem::Execute] : Weird amount of LevelRequestComponent.");

		for (LevelRequestComponent& component : *levelRequestContainer)
		{
			for (const LoadRequest& loadRequest : component.m_loadRequests)
			{
				HandleLoadRequest(loadRequest);
			}

			for (const SpawnRequest& spawnRequest : component.m_spawnRequests)
			{
				HandleSpawnRequest(spawnRequest);
			}

			component.m_loadRequests.clear();
			component.m_spawnRequests.clear();
		}
	}
	else // TODO : remove this hack
	{
		HandleLoadRequest(LoadRequest{ k_templatesFile, k_levelFile });
	}
}

void LevelLoadingSystem::HandleLoadRequest(const LoadRequest& request)
{
	InitializeComponentsStorage();

	LoadDataFile(request.templatesFilePath.c_str(), m_templatesFileBuffer, m_templatesDocument);
	LoadDataFile(request.levelFilePath.c_str(), m_levelFileBuffer, m_levelDocument);

	LoadComponentsDescription(m_templatesDocument.GetObject(), m_componentsDesc);
	LoadTemplatesDescription(m_templatesDocument.GetObject(), m_templatesDesc);

	LoadEntities(m_levelDocument.GetObject(), m_componentsDesc, m_templatesDesc);

	m_onLoadingComplete.Dispatch();
}

void LevelLoadingSystem::HandleSpawnRequest(const SpawnRequest& request)
{
	const auto FindTemplate = [&request](const Template& a) -> bool
	{
		return a.id == request.templateId;
	};

	const std::vector<Template>::const_iterator templateIt = std::find_if(m_templatesDesc.begin(), m_templatesDesc.end(), FindTemplate);

	if (templateIt != m_templatesDesc.end())
	{
		std::unordered_map<uint32_t, Entity>& entities = ECSManager::GetInstance()->GetEntities();
		const uint32_t entityId = entities.size();
		Entity& entity = entities[entityId];
		entity.m_entityId = entityId;

		const Template& templateDesc = *templateIt;

		InstantiateEntityFromTemplate(m_componentsDesc, templateDesc, request.name, entity);

		TransformComponentIterator& transformsContainer = *ECSManager::GetInstance()->GetComponents<TransformComponentIterator>();
		TransformComponent& component = transformsContainer.Add();

		component.m_entityId = entity.m_entityId;
		component.m_position = request.spawnPosition;
		component.m_rotation = request.spawnRotation;
		component.m_scale = {1.f, 1.f, 1.f};

		entity.m_components.insert({ComponentType::TransformComponent, &component});

		// TODO FIX HACK
		if (entity.m_components.contains(ComponentType::PhysicsComponent))
		{
			PhysicsComponent& phys = *entity.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();

			phys.m_position = request.spawnPosition;
			phys.m_rotation = request.spawnRotation;
		}

		m_onEntitySpawned.Dispatch(entityId);
	}
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

	RapidjsonUtils::ParseArrayValue(json, k_componentsDescValueName, false, elementCallback);
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

	RapidjsonUtils::ParseArrayValue(json, k_templatesValueName, false, elementCallback);
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
	RapidjsonUtils::ParseArrayValue(json, k_componentsValueName, false, elementCallback);

	return Template { id, name, components };
}

void LevelLoadingSystem::InitializeComponentsStorage()
{
	for (ComponentType type = static_cast<ComponentType>(0); type != ComponentType::Size; ++type)
	{
		if (Iterator* it = ECSManager::GetInstance()->GetComponents(type))
		{
			it->Clear();
		}
		else
		{
			IComponent::CreateIterator(type, Iterator::k_defaultCapacity);
		}
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

	RapidjsonUtils::ParseArrayValue(json, k_levelEntitiesValueName, false, elementCallback);
}

void LevelLoadingSystem::LoadEntity(const rapidjson::Value& json, const std::vector<ComponentDescription>& componentsDesc, const std::vector<Template>& templatesDesc, Entity& outEntity)
{
	ASSERT(json.HasMember(k_transformValueName) && json[k_transformValueName].IsObject(), "[LevelLoadingSystem::LoadEntity] : Invalid entities json");

	size_t templateId;
	std::string entityName;
	RapidjsonUtils::ParseStringValue(json, k_nameValueName, entityName);
	RapidjsonUtils::ParseUintValue(json, k_templateIdValueName, templateId);

	const auto templateIt = std::find_if(templatesDesc.begin(), templatesDesc.end(), [templateId](const Template& a) { return a.id == templateId; });
	ASSERT(templateIt != templatesDesc.end(), "[LevelLoadingSystem::LoadEntity] : Invalid entities json");

	InstantiateEntityFromTemplate(componentsDesc, *templateIt, entityName, outEntity);

	const rapidjson::Value& transformData = json[k_transformValueName];
	Iterator* transforms = ECSManager::GetInstance()->GetComponents(ComponentType::TransformComponent);
	const std::unique_ptr<ConstructionVisitor> cv = IComponent::CreateIteratorConstructionVisitor(ComponentType::TransformComponent, json[k_transformValueName]);
	transforms->Accept(cv, outEntity);
}

void LevelLoadingSystem::InstantiateEntityFromTemplate(const std::vector<ComponentDescription>& componentsDesc, const Template& templateDesc, const std::string& entityName, Entity& outEntity)
{
	outEntity.m_name = entityName;
	for (const auto componentId : templateDesc.componentIds)
	{
		const auto componentIt = std::find_if(componentsDesc.begin(), componentsDesc.end(), [componentId](const ComponentDescription& a) { return a.id == componentId; });
		ASSERT(componentIt != componentsDesc.end(), "[LevelLoadingSystem::LoadEntity] : Invalid entities json");

		const ComponentType compType = static_cast<ComponentType>(componentIt->type);

		Iterator* container = ECSManager::GetInstance()->GetComponents(compType);
		const std::unique_ptr<ConstructionVisitor> cv = IComponent::CreateIteratorConstructionVisitor(compType, componentIt->json);
		container->Accept(cv, outEntity);
	}
}

}
