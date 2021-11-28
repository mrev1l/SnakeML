#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class ParentSystem : public ISystemCastableImpl<ParentSystem>
{
public:
	ParentSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>()) : ISystemCastableImpl<ParentSystem>(targetEntities) {};
	virtual ~ParentSystem() = default;

	void Update(float dt) override;
};

}