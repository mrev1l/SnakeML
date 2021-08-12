#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class InitializeCameraSystem : public ISystem
{
public:
	InitializeCameraSystem() : ISystem() {}

	void Execute() override;
};

}
