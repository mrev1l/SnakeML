// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{
namespace system
{

class TransformComponent;
class PhysicsComponent;

class InitializeTransformComponentsSystem : public ISystem
{
public:
	void Execute() override;

	static void InitPhysicsBody(const TransformComponent& transform, PhysicsComponent& _outBody);
	static void InitAABB(PhysicsComponent& _outBody);
};

}
}