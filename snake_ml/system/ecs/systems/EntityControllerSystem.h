#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

// TODO : Rename into snake head controller?
class EntityControllerSystem : public ISystemCastableImpl<EntityControllerSystem>
{
public:
	EntityControllerSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>()) : ISystemCastableImpl<EntityControllerSystem>(targetEntities) {};
	virtual ~EntityControllerSystem() = default;

	void Update(float dt) override;

private:
	vector CalculateNewMovingDirection(vector currentMovingDirection, vector inputDirection);
	vector CalculateNewRotation(vector currentMovingDirection);

	static constexpr float s_maxMovementSpeed = 100.f;
};

}