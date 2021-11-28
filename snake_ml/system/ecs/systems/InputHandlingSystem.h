#pragma once
#include "system/ecs/ISystem.h"
#include "system/input/InputManager.h"

namespace snakeml
{

class InputHandlingSystem : public ISystemCastableImpl<InputHandlingSystem>
{
public:
	InputHandlingSystem(std::vector<uint32_t> targetEntities = std::vector<uint32_t>());

	void Update(float dt) override;

private:
	void OnInput(InputAction) const;
	void OnAxis(InputManager::InputAxisData) const;

	void Cheat_ToggleDebugRendering() const;
	void Cheat_SpawnSnakeSection() const;
	void PopulateInputVector(InputManager::InputAxisData) const;

	bool m_debugRenderingToggle = false; // TODO : Add macro for debug code
	uint32_t m_parentId = -1; // Add costexpr for invalid entity id
};

}