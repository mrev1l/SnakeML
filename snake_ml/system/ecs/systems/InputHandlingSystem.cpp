// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InputHandlingSystem.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/components/ChildControllerComponent.h"
#include "system/ecs/components/DebugRenderComponent.h"
#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/InputDataComponent.h"
#include "system/ecs/components/LevelRequestComponent.h"
#include "system/ecs/components/MaterialComponent.h"

#include "system/ecs/systems/InitializeDebugRenderComponentsSystem.h"
#include "system/ecs/systems/InitializePhysicsComponentsSystem.h"
#include "system/ecs/systems/platform_specific/win/InitializeRenderComponentsSystem.h"

namespace snakeml
{

InputHandlingSystem::InputHandlingSystem(std::vector<uint32_t> targetEntities) : ISystemCastableImpl<InputHandlingSystem>(targetEntities)
{
	InputManager::GetInstance()->m_onActionReleased.Subscribe(this, std::bind(&InputHandlingSystem::OnInput, this, std::placeholders::_1));
	InputManager::GetInstance()->m_onInputAxisEvent.Subscribe(this, std::bind(&InputHandlingSystem::OnAxis, this, std::placeholders::_1));
}

void InputHandlingSystem::Update(float dt)
{
	InputDataComponentIterator& it = *ECSManager::GetInstance()->GetComponents<InputDataComponentIterator>();
	ASSERT(it.Size() == 1u, "[InputHandlingSystem::PopulateInputVector] : Multiple entities storing input.");
	InputDataComponent& inputData = it.At(0);
	
	for (auto it = inputData.m_axesInput.rbegin(); it != inputData.m_axesInput.rend(); ++it)
	{
		InputManager::InputAxisData inputAxis = *it;
		if (!IsNearlyZero(inputAxis.value))
		{
			EntityControllerComponentIterator& it = *ECSManager::GetInstance()->GetComponents<EntityControllerComponentIterator>();
			ASSERT(it.Size() == 1u, "[InputHandlingSystem::Update] : Multiple entities receiving input.");

			const vector inputDirection = inputAxis.axis == InputAxis::MoveForward ? vector::up : vector::right;
			it.At(0).m_inputVector += inputDirection * inputAxis.value;
		}
	}
	inputData.m_axesInput.clear();
}

void InputHandlingSystem::OnInput(InputAction inputEvent) const
{
	switch (inputEvent)
	{
	case InputAction::DebugRendering: Cheat_ToggleDebugRendering(); break;
	case InputAction::DebugSpawnSnakeChild: Cheat_SpawnSnakeSection(); break;
	default:
		break;
	}
}

void InputHandlingSystem::OnAxis(InputManager::InputAxisData inputAxis) const
{
	switch (inputAxis.axis)
	{
	case InputAxis::MoveForward:
	case InputAxis::MoveRight:		PopulateInputVector(inputAxis); break;
	}
}

void InputHandlingSystem::Cheat_ToggleDebugRendering() const
{
	InputHandlingSystem& me = const_cast<InputHandlingSystem&>(*this);
	me.m_debugRenderingToggle = !me.m_debugRenderingToggle;
	DebugRenderComponentIterator* it = ECSManager::GetInstance()->GetComponents<DebugRenderComponentIterator>();
	for (DebugRenderComponent& debugRenderComp : *it)
	{
		debugRenderComp.m_isEnabled = me.m_debugRenderingToggle;
	}
}

void InputHandlingSystem::Cheat_SpawnSnakeSection() const
{
	static size_t s_count = 0;
	if(m_parentId == -1)
	{
		const std::unordered_map<uint32_t, Entity>& entities = ECSManager::GetInstance()->GetEntities();
		for (const std::pair<uint32_t, Entity>& entity : entities)
		{
			if (entity.second.m_components.contains(ComponentType::EntityControllerComponent))
			{
				InputHandlingSystem* me = const_cast<InputHandlingSystem*>(this);
				me->m_parentId = entity.first;
				break;
			}
		}
	}

	LevelRequestComponentIterator* levelRequestContainer = ECSManager::GetInstance()->GetComponents<LevelRequestComponentIterator>();
	if (levelRequestContainer)
	{
		ASSERT(levelRequestContainer->Size() <= 1, "[InputHandlingSystem::Cheat_SpawnSnakeSection] : Weird amount of LevelRequestComponent.");

		levelRequestContainer->At(0).m_spawnRequests.push_back(
			LevelLoadingSystem::SpawnRequest{
				5u,															// templateId
				std::string("Snake_child_") + std::to_string(++s_count),	// name
				vector::zero,												// spawnPosition
				vector::zero												// spawnRotation
			}
		);

		LevelLoadingSystem* loadingSystem = ECSManager::GetInstance()->GetSystem<LevelLoadingSystem>();
		const auto OnSpawned = [this, loadingSystem](uint32_t spawnedId) -> void
		{
			InputHandlingSystem* me = const_cast<InputHandlingSystem*>(this);
			Entity& spawned = ECSManager::GetInstance()->GetEntity(spawnedId);

			ASSERT(spawned.m_components.contains(ComponentType::ChildControllerComponent), "[InputHandlingSystem::Cheat_SpawnSnakeSection] : spawned entity is set up incorrectly.");

			spawned.m_components.at(ComponentType::ChildControllerComponent)->As<ChildControllerComponent>()->m_parentId = me->m_parentId;
			me->m_parentId = spawnedId;

			std::vector<uint32_t> entitiesToInit = { spawnedId };

			std::unique_ptr<InitializePhysicsComponentsSystem> initPhysicsCompSystem = std::make_unique<InitializePhysicsComponentsSystem>(entitiesToInit);
			std::unique_ptr<InitializeDebugRenderComponentsSystem> initDebugRenderSystem = std::make_unique<InitializeDebugRenderComponentsSystem>(entitiesToInit);
			std::unique_ptr<win::InitializeRenderComponentsSystem> initRenderSystem = std::make_unique<win::InitializeRenderComponentsSystem>(entitiesToInit);

			ECSManager::GetInstance()->ExecuteSystem(std::move(initPhysicsCompSystem));
			ECSManager::GetInstance()->ExecuteSystem(std::move(initDebugRenderSystem));
			ECSManager::GetInstance()->ExecuteSystem(std::move(initRenderSystem));

			spawned.m_components.at(ComponentType::DebugRenderComponent)->As<DebugRenderComponent>()->m_isEnabled = me->m_debugRenderingToggle;
			spawned.m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>()->m_textureId = MaterialComponent::TextureId::Straight;

			loadingSystem->m_onEntitySpawned.Unsubscribe(const_cast<InputHandlingSystem*>(me));
		};

		loadingSystem->m_onEntitySpawned.Subscribe(const_cast<InputHandlingSystem*>(this), OnSpawned); // TODO : add implementation for subscriber to be const void*
	}

}

void InputHandlingSystem::PopulateInputVector(InputManager::InputAxisData inputAxis) const
{
	InputDataComponentIterator& it = *ECSManager::GetInstance()->GetComponents<InputDataComponentIterator>();
	ASSERT(it.Size() == 1u, "[InputHandlingSystem::PopulateInputVector] : Multiple entities storing input.");

	it.At(0).m_axesInput.push_back(inputAxis);
}

}