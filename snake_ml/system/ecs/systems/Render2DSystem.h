// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{

class Render2DSystem : public ISystem
{
public:
	Render2DSystem() : ISystem() {}

	void Update(float deltaTime) override;
};

}
