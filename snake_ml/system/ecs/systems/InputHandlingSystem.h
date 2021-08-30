#pragma once
#include "system/ecs/ISystem.h"
#include "system/input/InputManager.h"

namespace snakeml
{

class InputHandlingSystem : public ISystem
{
public:
	InputHandlingSystem();

	void Update(float dt) override;

private:
	void OnInput(InputAction) const;
	void OnAxis(InputManager::InputAxisData) const;

	void ToggleDebugRendering() const;
	void PopulateInputVector(InputManager::InputAxisData) const;
};

}