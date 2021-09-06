#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class InitializeDebugRenderComponentsSystem : public ISystemCastableImpl<InitializeDebugRenderComponentsSystem>
{
public:
	InitializeDebugRenderComponentsSystem() = default;

	void Execute() override;
};

}