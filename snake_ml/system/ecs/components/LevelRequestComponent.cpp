// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "LevelRequestComponent.h"

#include "utils/rapidjson_utils.h"

namespace snakeml
{

void LevelRequestComponentConstructionVisitor::Visit(Iterator* it, Entity& entity)
{
	LevelRequestComponentIterator& container = *it->As<LevelRequestComponentIterator>();
	LevelRequestComponent& component = container.Add();

	InitSpawnRequests(m_description, component.m_spawnRequests);
	InitLoadRequests(m_description, component.m_loadRequests);

	AttachComponentToEntity(component, entity);
}

void LevelRequestComponentConstructionVisitor::InitSpawnRequests(const rapidjson::Value& json, std::vector<LevelLoadingSystem::SpawnRequest>& _outSpawnRequests)
{
	// implement
}

void LevelRequestComponentConstructionVisitor::InitLoadRequests(const rapidjson::Value& json, std::vector<LevelLoadingSystem::LoadRequest>& _outLoadRequests)
{
	const auto elementCallback = [&_outLoadRequests](const rapidjson::Value* it) -> void
	{
		LevelLoadingSystem::LoadRequest request;

		RapidjsonUtils::ParseStringValue(*it, k_templatesPathValueName, request.templatesFilePath);
		RapidjsonUtils::ParseStringValue(*it, k_levelPathValueName, request.levelFilePath);

		_outLoadRequests.push_back(request);
	};

	RapidjsonUtils::ParseArrayValue(json, k_loadRequestsValueName, true, elementCallback);
}

}