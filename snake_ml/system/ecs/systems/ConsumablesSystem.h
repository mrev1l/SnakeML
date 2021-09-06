#pragma once

#include "system/ecs/ISystem.h"
#include "system/ecs/systems/PhysicsSystem.h"

namespace snakeml
{

class ConsumablesSystem : public ISystemCastableImpl<ConsumablesSystem>
{
public:
	ConsumablesSystem();

private:
	void OnCollisionEvent(Collision);
	static bool VerifyCollision(Collision);
	static bool DoesEntityContainComponent(uint32_t entityId, ComponentType componentType);
};


}