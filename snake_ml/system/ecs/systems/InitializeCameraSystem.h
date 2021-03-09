#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{
namespace system
{

class InitializeCameraSystem : public ISystem
{
public:
	void Execute() override;
};

}
}