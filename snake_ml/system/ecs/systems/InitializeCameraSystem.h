#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{
namespace system
{

class InitializeCameraSystem : public ISystem
{
public:
	InitializeCameraSystem() : ISystem() {}

	void Execute() override;
};

}
}