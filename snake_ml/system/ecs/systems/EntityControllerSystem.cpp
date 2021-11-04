// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "EntityControllerSystem.h"

#include "system/ecs/components/EntityControllerComponent.h"
#include "system/ecs/components/PhysicsComponent.h"
#include "system/ecs/components/TransformComponent.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

namespace snakeml
{

void EntityControllerSystem::Update(float dt)
{
	EntityControllerComponentIterator& entityControllers = *ECSManager::GetInstance()->GetComponents<EntityControllerComponentIterator>();
	for (EntityControllerComponent& controller : entityControllers)
	{
		Entity& entity = ECSManager::GetInstance()->GetEntity(controller.m_entityId);
		PhysicsComponent& physics = *entity.m_components.at(ComponentType::PhysicsComponent)->As<PhysicsComponent>();
		//TransformComponent& transform = *entity.m_components.at(ComponentType::TransformComponent)->As<TransformComponent>();

		const vector normalizedInput = controller.m_inputVector.getNormalized();
		//if (!IsNearlyZero(normalizedInput.length()))
		{
			static float timer = 0.f;
			static constexpr float timerThreshold = 0.5f;
			static vector forwardToApply = vector::zero;

			timer += dt;

			if (timer < timerThreshold)
			{
				physics.m_velocity = vector::zero;
				if (!IsNearlyZero(normalizedInput.length()))
				{
					matrix rotationMatrix = RotationMatrix(ConvertToRadians(physics.m_rotation.y), ConvertToRadians(physics.m_rotation.x), ConvertToRadians(physics.m_rotation.z));
					const vector currentForward = rotationMatrix * vector::up;

					const vector newForward = CalculateNewMovingDirection(currentForward, normalizedInput);
					forwardToApply = newForward;
				}
			}
			else
			{
				timer = 0.f;

				if (!IsNearlyZero(forwardToApply.length()))
				{
					physics.m_velocity = forwardToApply * /*s_maxMovementSpeed*/32.f / dt;
					physics.m_rotation = CalculateNewRotation(forwardToApply);

					//forwardToApply = vector::zero;
				}
			}
			controller.m_inputVector = vector::zero;
			
			//const vector currentVelocityDirection = physics.m_velocity.getNormalized();
			//const vector newMovementDirection = CalculateNewMovingDirection(currentVelocityDirection, normalizedInput);

			//physics.m_velocity = newMovementDirection * /*s_maxMovementSpeed*/32.f / dt;
			//physics.m_rotation = CalculateNewRotation(newMovementDirection);

			////transform.m_rotation = physics.m_rotation;

			//controller.m_inputVector = vector::zero;
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