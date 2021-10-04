#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

class CameraComponent : public IComponentCastableImpl<CameraComponent>
{
public:
	virtual ~CameraComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::CameraComponent; }

	vector m_eyePosition;
	vector m_focusPoint;
	vector m_upDirection;
};

class CameraComponentConstructionVisitor : public ConstructionVisitor
{
public:
	CameraComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::CameraComponent; }
	void Visit(Iterator* it, Entity& entity) override;

private:
	static constexpr const char* k_eyePositionValueName	= "eyePosition";
	static constexpr const char* k_focusPointValueName	= "focusPoint";
	static constexpr const char* k_upDirectionValueName	= "upDirection";
};

REGISTER_COMPONENT(CameraComponent);

}
