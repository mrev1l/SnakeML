#pragma once

#include "system/ecs/IComponent.h"
#include "utils/math/algorithms/AABB.h"

namespace snakeml
{

enum class CollisionChannel : uint32_t
{
	Static	= 1 << 0,
	Dynamic	= 1 << 1,

	None	= 0
};
inline constexpr CollisionChannel operator|(CollisionChannel a, CollisionChannel b)
{
	return static_cast<CollisionChannel>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline constexpr CollisionChannel operator&(CollisionChannel a, CollisionChannel b)
{
	return static_cast<CollisionChannel>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
static constexpr CollisionChannel CollisionChannel_ALL = CollisionChannel::Static | CollisionChannel::Dynamic;

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

	BoxShape m_shape					= BoxShape();
	AABB m_aabb							= AABB{ vector::zero, vector::zero };
	vector m_position					= vector::zero;
	vector m_rotation					= vector::zero;
	vector m_velocity					= vector::zero;
	vector m_angularVelocity			= vector::zero;
	vector m_acceleration				= vector::zero;
	vector m_angularAcceleration		= vector::zero;
	// TODO : Deprecate ?
	bool m_isDynamic					= false;
	CollisionChannel m_collisionChannel	= CollisionChannel::None;
	CollisionChannel m_collisionFilter	= CollisionChannel_ALL;
};

class PhysicsComponentConstructionVisitor : public ConstructionVisitor
{
public:
	PhysicsComponentConstructionVisitor(const rapidjson::Value& json) : ConstructionVisitor(json) {}
	ComponentType GetReceiverType() override { return ComponentType::PhysicsComponent; }

	void Visit(Iterator* it, Entity& entity) override;

private:
	static constexpr const char* k_shapeDimensionsValueName		= "shape_dimensions";
	static constexpr const char* k_shapeMassValueName			= "shape_mass";
	static constexpr const char* k_isDynamicValueName			= "is_dynamic";
	static constexpr const char* k_collisionChannelValueName	= "collision_channel";
	static constexpr const char* k_collisionFilterValueName		= "collision_filter";
};

REGISTER_COMPONENT(PhysicsComponent);

}
