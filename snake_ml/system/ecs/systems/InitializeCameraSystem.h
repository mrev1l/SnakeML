#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class InitializeCameraSystem : public ISystemCastableImpl<InitializeCameraSystem>
{
public:
	InitializeCameraSystem() = default;

	void Execute() override;
};

}
