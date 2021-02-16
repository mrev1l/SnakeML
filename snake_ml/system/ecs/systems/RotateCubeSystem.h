// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/ISystem.h"

namespace snakeml
{
namespace system
{

class RotateCubeSystem : public ISystem
{
public:
	void Execute() override;
	//{
	//	static std::chrono::high_resolution_clock clock;
	//	static auto t0 = clock.now();

	//	auto t1 = clock.now(); //-V656
	//	auto deltaTime = t1 - t0;

	//	const float angle = static_cast<float>(deltaTime.count() * 1e-9 * 90.0);

	//	const uint32_t entityIdToUpdate = 0;

	//	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	//	const std::vector<Entity>::const_iterator entityIt = std::find_if(entities.begin(), entities.end(),
	//		[entityIdToUpdate](const Entity& a) {return a.m_entityId == entityIdToUpdate; });
	//	if (entityIt != entities.end())
	//	{
	//		TransformComponent& transform = *(TransformComponent*)(entityIt->m_components.at(ComponentType::TransformComponent));
	//		auto scale = DirectX::XMMatrixScalingFromVector({ transform.m_scale.m_x, transform.m_scale.m_y, transform.m_scale.m_z, transform.m_scale.m_w });
	//		auto rotation = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(transform.m_rotation.m_x), DirectX::XMConvertToRadians(transform.m_rotation.m_y), DirectX::XMConvertToRadians(transform.m_rotation.m_z));
	//		auto translation = DirectX::XMMatrixTranslationFromVector({ transform.m_position.m_x, transform.m_position.m_y, transform.m_position.m_z, transform.m_position.m_w });
	//		auto transformMatrix = scale * rotation * translation;

	//		auto updatedRotation = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(0.f), DirectX::XMConvertToRadians(0.f), DirectX::XMConvertToRadians(angle));
	//		transformMatrix = transformMatrix * updatedRotation;

	//		DirectX::XMVECTOR newScale, newRotation, newTranslation;
	//		DirectX::XMMatrixDecompose(&newScale, &newRotation, &newTranslation, transformMatrix);

	//		DirectX::XMVECTOR pitchAxis = {1.0f, 0.f, 0.f, 0.f};
	//		float pitchAngle = 0.f;
	//		DirectX::XMVECTOR yawAxis = {0.0f, 1.f, 0.f, 0.f};
	//		float yawAngle = 0.f;
	//		DirectX::XMVECTOR rollAxis = {0.0f, 0.f, 1.f, 0.f};
	//		float rollAngle = 0.f;
	//		DirectX::XMQuaternionToAxisAngle(&yawAxis, &pitchAngle, newRotation);
	//		DirectX::XMQuaternionToAxisAngle(&yawAxis, &yawAngle, newRotation);
	//		DirectX::XMQuaternionToAxisAngle(&yawAxis, &rollAngle, newRotation);

	//		float newPitchAngle = DirectX::XMConvertToDegrees(pitchAngle);
	//		float newYawAngle = DirectX::XMConvertToDegrees(yawAngle);
	//		float newRollAngle = DirectX::XMConvertToDegrees(rollAngle);

	//		int stop = 23;
	//	}
	//}
};

}
}