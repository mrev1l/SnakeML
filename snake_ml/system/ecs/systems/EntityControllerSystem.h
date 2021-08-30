#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class EntityControllerSystem : public ISystem
{
public:
	EntityControllerSystem() = default;
	virtual ~EntityControllerSystem() = default;

	void Update(float dt) override;

private:
	vector CalculateNewMovingDirection(vector currentMovingDirection, vector inputDirection);

	static constexpr float s_maxMovementSpeed = 100.f;
};

}