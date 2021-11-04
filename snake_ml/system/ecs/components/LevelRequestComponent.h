#pragma once

#include "system/ecs/IComponent.h"
#include "system/ecs/systems/LevelLoadingSystem.h"

namespace snakeml
{

class LevelRequestComponent : public IComponentCastableImpl<LevelRequestComponent>
{
public:
	virtual ~LevelRequestComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::LevelRequestComponent; }

	std::vector<LevelLoadingSystem::SpawnRequest> m_spawnRequests;
	std::vector<LevelLoadingSystem::LoadRequest> m_loadRequests;
};

class LevelRequestComponentConstructionVisitor : public ConstructionVisitor
{
public:
	LevelRequestComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::LevelRequestComponent; }
	void Visit(Iterator* it, Entity& entity) override;

private:
	static void InitSpawnRequests(const rapidjson::Value& json, std::vector<LevelLoadingSystem::SpawnRequest>& _outSpawnRequests);
	static void InitLoadRequests(const rapidjson::Value& json, std::vector<LevelLoadingSystem::LoadRequest>& _outLoadRequests);

	static constexpr const char* k_spawnRequestsValueName = "spawnRequests";

	static constexpr const char* k_loadRequestsValueName	= "loadRequests";
	static constexpr const char* k_templatesPathValueName	= "templatesPath";
	static constexpr const char* k_levelPathValueName		= "levelPath";
};

REGISTER_COMPONENT(LevelRequestComponent);

}