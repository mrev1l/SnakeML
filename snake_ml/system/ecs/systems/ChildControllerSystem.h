#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class ChildControllerSystem : public ISystemCastableImpl<ChildControllerSystem>
{
public:
	ChildControllerSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>()) : ISystemCastableImpl<ChildControllerSystem>(targetEntities) {};
	virtual ~ChildControllerSystem() = default;

	void Update(float dt) override;

};

}