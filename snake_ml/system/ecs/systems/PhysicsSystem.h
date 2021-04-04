#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{
namespace system
{

class PhysicsComponent;

class PhysicsSystem : public ISystem
{
public:
	void Update(double deltaTime) override;

private:
	void UpdateAABB(PhysicsComponent& body);
};

}
}