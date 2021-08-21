#pragma once

#include "system/ecs/IComponent.h"
#include "utils/math/algorithms/AABB.h"

namespace snakeml
{

struct BoxShape
{
	vector m_dimensions		= vector::zero;
	float m_mass			= 0.f;
	float m_momentOfInertia	= 0.f; // m * (w*w + h*h) / 12.f
};

class PhysicsComponent : public IComponentCastableImpl<PhysicsComponent>
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
