// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12RenderCommandFactory.h"

#include "DrawMeshCommand.h"
#include "SetupSimplePixelShaderCommand.h"

#include "system/drivers/win/dx/DX12CommandList.h"
#include "system/drivers/win/dx/DX12Driver.h"

#include "system/ecs/components/platform_specific/win/DX12RenderComponent.h"
#include "system/ecs/components/CameraComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

void DX12RenderCommandFactory::BuildRenderCommands(const Entity& entity, std::vector<std::unique_ptr<IRenderCommand>>& outRenderCommands)
{
	CameraComponent& camera = *(CameraComponent*)ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::CameraComponent)->At(0);
	DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(camera.m_eyePosition.m_x, camera.m_eyePosition.m_y, camera.m_eyePosition.m_z, camera.m_eyePosition.m_w),
		DirectX::XMVectorSet(camera.m_focusPoint.m_x, camera.m_focusPoint.m_y, camera.m_focusPoint.m_z, camera.m_focusPoint.m_w),
		DirectX::XMVectorSet(camera.m_upDirection.m_x, camera.m_upDirection.m_y, camera.m_upDirection.m_z, camera.m_upDirection.m_w));

	const DX12RenderComponent& renderableComponent = *(DX12RenderComponent*)entity.m_components.at(ComponentType::DX12RenderComponent);
	const TransformComponent& transformComponent = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);

	DirectX::XMMATRIX projection, orthogonal;
	win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
	dx12Driver->GetMatrices(projection, orthogonal);

	auto scaleMatrix = DirectX::XMMatrixScaling(transformComponent.m_scale.m_x, transformComponent.m_scale.m_y, transformComponent.m_scale.m_z);
	auto rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(transformComponent.m_rotation.m_x), DirectX::XMConvertToRadians(transformComponent.m_rotation.m_y), DirectX::XMConvertToRadians(transformComponent.m_rotation.m_z));
	auto translationMatrix = DirectX::XMMatrixTranslation(transformComponent.m_position.m_x, transformComponent.m_position.m_y, transformComponent.m_position.m_z);

	const DirectX::XMMATRIX modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	DirectX::XMMATRIX mvpMatrix = modelMatrix * cameraMatrix * orthogonal;

	outRenderCommands.push_back(std::make_unique<SetupSimplePixelShaderCommand>(renderableComponent.m_pipelineState, renderableComponent.m_rootSignature, renderableComponent.m_texture, mvpMatrix));
	outRenderCommands.push_back(std::make_unique<DrawMeshCommand>(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, renderableComponent.m_vertexBuffer));
}

}
#endif
}
}