// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class TransformComponent : public IComponentCastableImpl<TransformComponent>
{
public:
	virtual ~TransformComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::TransformComponent; }

	vector m_position;
	vector m_rotation;
	vector m_scale;
};

class TransformComponentConstructionVisitor : public ConstructionVisitor
{
public:
	TransformComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::TransformComponent; }

	void Visit(Iterator* it, Entity& entity) override;

private:
	static constexpr const char* k_positionValueName	= "position";
	static constexpr const char* k_rotationValueName	= "rotation";
	static constexpr const char* k_scaleValueName		= "scale";
};

REGISTER_COMPONENT(TransformComponent);

}
