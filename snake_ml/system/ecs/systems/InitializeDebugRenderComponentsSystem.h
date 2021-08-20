#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class InitializeDebugRenderComponentsSystem : public ISystem
{
public:
	InitializeDebugRenderComponentsSystem() = default;

	void Execute() override;
};

}