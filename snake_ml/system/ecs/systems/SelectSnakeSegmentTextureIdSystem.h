#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class SelectSnakeSegmentTextureIdSystem : public ISystemCastableImpl<SelectSnakeSegmentTextureIdSystem>
{
public:
	SelectSnakeSegmentTextureIdSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>()) : ISystemCastableImpl<SelectSnakeSegmentTextureIdSystem>(targetEntities) {};
	virtual ~SelectSnakeSegmentTextureIdSystem() = default;

	//void Update(float dt) override;
	void Execute() override;
};

}