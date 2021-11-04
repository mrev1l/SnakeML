// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class TransformComponent;
class PhysicsComponent;

class InitializePhysicsComponentsSystem : public ISystemCastableImpl<InitializePhysicsComponentsSystem>
{
public:
	InitializePhysicsComponentsSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>()) : ISystemCastableImpl<InitializePhysicsComponentsSystem>(targetEntities) {};

	void Execute() override;

private:
	static void InitPhysicsBody(const TransformComponent& transform, PhysicsComponent& _outBody);
};

}
