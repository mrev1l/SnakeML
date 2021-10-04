// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"

#include "utils/types/arithmetic.h"

namespace snakeml
{

class MaterialComponent : public IComponentCastableImpl<MaterialComponent>
{
public:
	enum class InputLayoutEntries
	{
		Position,
		Color,
		UV,

		Count
	};

	virtual ~MaterialComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::MaterialComponent; }

	std::wstring m_vs;
	std::wstring m_ps;
	std::vector<InputLayoutEntries> m_inputLayoutEntries;
	std::wstring m_texturePath;
};

class MaterialComponentConstructionVisitor : public ConstructionVisitor
{
public:
	MaterialComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::MaterialComponent; }

	void Visit(Iterator*, Entity& entity) override;

private:
	static void InitInputLayoutDescription(const rapidjson::Value& json, std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries);

	static constexpr const char* k_vsValueName		= "vs";
	static constexpr const char* k_psValueName		= "ps";
	static constexpr const char* k_textureValueName	= "texture";
};

REGISTER_COMPONENT(MaterialComponent);

}
