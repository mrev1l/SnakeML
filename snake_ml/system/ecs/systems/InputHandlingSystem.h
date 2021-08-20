#pragma once
#include "system/ecs/ISystem.h"
#include "system/input/InputKeys.h"

namespace snakeml
{

class InputHandlingSystem : public ISystem
{
public:
	InputHandlingSystem();

private:
	void OnInput(InputKey inputKey);
	void ToggleDebugRendering();
};

}