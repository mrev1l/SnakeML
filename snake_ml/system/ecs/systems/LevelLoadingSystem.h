// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/components/MaterialComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/ISystem.h"

namespace snakeml
{

class LevelLoadingSystem : public ISystemCastableImpl<LevelLoadingSystem>
{
public:
	LevelLoadingSystem() = default;

	void Execute() override;

private:
	struct ComponentDescription
	{
		ComponentDescription(size_t _id, ComponentType _type, const rapidjson::Value& _json) : id(_id), type(_type), json(_json) {}
		size_t id;
		ComponentType type;
		const rapidjson::Value& json;
	};

	struct Template
	{
		Template(size_t _id, const std::string& _name, const std::vector<size_t> _components) : id(_id), name(_name), componentIds(_components) {}
		size_t id;
		std::string name;
		std::vector<size_t> componentIds;
	};

	static void LoadDataFile(const char* fileName, std::string& _outFileBuffer, rapidjson::Document& _outFileDocument);

	static void LoadComponentsDescription(const rapidjson::Value& json, std::vector<ComponentDescription>& outComponentsDesc);
	static ComponentDescription LoadComponentDescription(const rapidjson::Value& json);

	static void LoadTemplatesDescription(const rapidjson::Value& json, std::vector<Template>& outTemplates);
	static Template LoadTemplateDescription(const rapidjson::Value& json);

	static void InitializeComponentsStorage();

	static void LoadEntities(const rapidjson::Value& json, const std::vector<ComponentDescription>& componentsDesc, const std::vector<Template>& templatesDesc);
	static void LoadEntity(const rapidjson::Value& json, const std::vector<ComponentDescription>& componentsDesc, const std::vector<Template>& templatesDesc, Entity& outEntity);

	std::string m_levelFileBuffer, m_templatesFileBuffer;
	rapidjson::Document m_levelDocument, m_templatesDocument;

	std::vector<ComponentDescription> m_componentsDesc;
	std::vector<Template> m_templatesDesc;

	static constexpr const char* k_levelFile		= "data\\assets\\levels\\level_0\\level_0.txt";
	static constexpr const char* k_templatesFile	= "data\\assets\\levels\\templates\\templates.txt";

	static constexpr const char* k_componentsDescValueName	= "component_desription";
	static constexpr const char* k_descriptionValueName		= "description";
	static constexpr const char* k_componentIdValueName		= "componentId";
	static constexpr const char* k_componentTypeIdValueName	= "componentTypeId";

	static constexpr const char* k_templatesValueName	= "templates";
	static constexpr const char* k_templateIdValueName	= "templateId";
	static constexpr const char* k_nameValueName		= "name";
	static constexpr const char* k_componentsValueName	= "components";

	static constexpr const char* k_levelEntitiesValueName	= "level_entities";
	static constexpr const char* k_transformValueName		= "transform";
	
};

}
