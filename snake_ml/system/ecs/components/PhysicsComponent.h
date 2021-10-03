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
	ComponentType GetReceiverType() { return ComponentType::PhysicsComponent; }

	void Visit(Iterator* it, Entity& entity) override;

private:
	static void ParsePhysicsComponents_ShapeDimensions(const rapidjson::Value& json, vector& _outShapeDimensions);
	static void ParsePhysicsComponents_ShapeMass(const rapidjson::Value& json, float& _outMass);
	static void ParsePhysicsComponents_IsDynamic(const rapidjson::Value& json, bool& _outIsDynamic);
	static void ParsePhysicsComponents_CollisionChannel(const rapidjson::Value& json, CollisionChannel& _outCollisionChannel);
	static void ParsePhysicsComponents_CollisionFilter(const rapidjson::Value& json, CollisionChannel& _outCollisionFilter);
};

REGISTER_COMPONENT(PhysicsComponent);

}
