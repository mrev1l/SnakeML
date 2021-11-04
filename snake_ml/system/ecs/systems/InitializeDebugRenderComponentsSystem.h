#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class DebugRenderComponent;
class Entity;
class PhysicsComponent;

class InitializeDebugRenderComponentsSystem : public ISystemCastableImpl<InitializeDebugRenderComponentsSystem>
{
public:
	InitializeDebugRenderComponentsSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>()) : ISystemCastableImpl<InitializeDebugRenderComponentsSystem>(targetEntities) {};

	void Execute() override;

private:
	static void InitDebugRenderComponent(Entity& entity, const PhysicsComponent& physicsComp, DebugRenderComponent& debugRenderComponent);
};

}