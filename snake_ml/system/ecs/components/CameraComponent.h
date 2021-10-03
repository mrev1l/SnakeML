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
	ComponentType GetReceiverType() { return ComponentType::CameraComponent; }
	void Visit(Iterator* it, Entity& entity) override;

private:
	static void ParseVectorValue(const rapidjson::Value& json, const char* name, vector& _outVector);
};

REGISTER_COMPONENT(CameraComponent);

}
