#pragma once

#include "system/ecs/IComponent.h"

namespace snakeml
{
namespace system
{

struct BoxShape
{
	math::vector m_dimensions;
	float m_mass;
	float m_momentOfInertia; // m * (w*w + h*h) / 12.f
};

struct AABB
{
	math::vector min, max;
};

class PhysicsComponent : public IComponent
{
public:
	virtual ~PhysicsComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::PhysicsComponent; }

	BoxShape m_shape;
	types::AABB m_aabb;
	math::vector m_position;
	math::vector m_rotation;
	math::vector m_velocity;
	math::vector m_angularVelocity;
	math::vector m_acceleration;
	math::vector m_angularAcceleration;
	bool m_isDynamic = false;
};
REGISTER_TYPE(PhysicsComponent);

}
}