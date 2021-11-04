#pragma once

#include "system/ecs/ISystem.h"
#include "system/ecs/systems/PhysicsSystem.h"

namespace snakeml
{

class ConsumablesSystem : public ISystemCastableImpl<ConsumablesSystem>
{
public:
	ConsumablesSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>());

private:
	void OnCollisionEvent(Collision);
	static bool VerifyCollision(Collision);
	static bool DoesEntityContainComponent(uint32_t entityId, ComponentType componentType);
};


}