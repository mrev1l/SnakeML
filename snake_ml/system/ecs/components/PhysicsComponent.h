#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{

struct BoxShape
{
	vector m_dimensions;
	float m_mass;
	float m_momentOfInertia; // m * (w*w + h*h) / 12.f
};

class PhysicsComponent : public IComponent
{
public:
	virtual ~PhysicsComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::PhysicsComponent; }

	BoxShape m_shape;
	AABB m_aabb;
	vector m_position;
	vector m_rotation;
	vector m_velocity;
	vector m_angularVelocity;
	vector m_acceleration;
	vector m_angularAcceleration;
	bool m_isDynamic = false;
};
REGISTER_TYPE(PhysicsComponent);

}
