// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "EntityControllerSystem.h"

#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/PhysicsComponent.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

namespace snakeml
{

void EntityControllerSystem::Update(float dt)
{
	EntityControllerComponentIterator& entityControllers = *ECSManager::GetInstance()->GetComponents<EntityControllerComponentIterator>();
	for (EntityControllerComponent& controller : entityControllers)
	{
		Entity entity = ECSManager::GetInstance()->GetEntity(controller.m_entityId);
		PhysicsComponent& physics = *entity.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();

		const vector normalizedInput = controller.m_inputVector.getNormalized();
		if (!IsNearlyZero(normalizedInput.length()))
		{
			const vector currentVelocityDirection = physics.m_velocity.getNormalized();
			const vector newMovementDirection = CalculateNewMovingDirection(currentVelocityDirection, normalizedInput);

			physics.m_velocity = newMovementDirection * s_maxMovementSpeed;
			physics.m_rotation = CalculateNewRotation(newMovementDirection);

			controller.m_inputVector = vector::zero;
		}
	}
}

vector EntityControllerSystem::CalculateNewMovingDirection(vector currentMovingDirection, vector inputDirection)
{
	if (currentMovingDirection.length() == 0.f)
	{
		return inputDirection;
	}

	const float velocityInputDot = currentMovingDirection.dot(inputDirection);
	if (!IsNearlyZero(velocityInputDot))
	{
		return currentMovingDirection;
	}
	const vector currentMovingDirTangent = perpendicular2d(currentMovingDirection);
	const float velocityTangentInputDot = currentMovingDirTangent.dot(inputDirection);
	if (velocityTangentInputDot > 0.f)
	{
		return currentMovingDirTangent;
	}
	else
	{
		return -currentMovingDirTangent;
	}
}

vector EntityControllerSystem::CalculateNewRotation(vector currentMovingDirection)
{
	vector result = vector::zero;

	const float movementUpDot = currentMovingDirection.dot(vector::up);
	if (!IsNearlyZero(movementUpDot))
	{
		result.z = ConvertToDegrees(acosf(movementUpDot));
	}
	else
	{
		const vector movementTangent = perpendicular2d(currentMovingDirection);
		const float movementTangetUpDot = movementTangent.dot(vector::up);
		result.z = ConvertToDegrees(acosf(movementTangetUpDot)) - 90.f;
	}

	return result;
}

}