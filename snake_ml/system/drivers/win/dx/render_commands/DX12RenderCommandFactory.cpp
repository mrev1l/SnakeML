// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12RenderCommandFactory.h"

#include "DrawMeshCommand.h"
#include "SetupAABBPixelShaderCommand.h"
#include "SetupSimplePixelShaderCommand.h"

#include "system/drivers/win/dx/DX12Driver.h"

#include "system/ecs/components/platform_specific/win/DX12RenderComponent.h"
#include "system/ecs/components/CameraComponent.h"
#include "system/ecs/components/DebugRenderComponent.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

void DX12RenderCommandFactory::BuildRenderCommands(const Entity& entity, std::vector<std::unique_ptr<IRenderCommand>>& _outRenderCommands)
{
	matrix projection, orthogonal;
	matrix scaleMatrix, rotationMatrix, translationMatrix;
	const matrix cameraMatrix = BuildRenderCommands_CalculateCameraMatrix();
	((win::DX12Driver*)IRenderDriver::GetInstance())->GetMatrices(projection, orthogonal);
	BuildRenderCommands_CalculateEntityMatrices(entity, scaleMatrix, rotationMatrix, translationMatrix);

	BuildRenderCommands_Main(entity, cameraMatrix, orthogonal, scaleMatrix, rotationMatrix, translationMatrix, _outRenderCommands);
	BuildRenderCommands_Debug(entity, cameraMatrix, orthogonal, translationMatrix, _outRenderCommands);
}

matrix DX12RenderCommandFactory::BuildRenderCommands_CalculateCameraMatrix()
{
	CameraComponent& camera = ECSManager::GetInstance()->GetComponents<CameraComponentIterator>()->At(0);
	return LookAtMatrixLH(camera.m_eyePosition, camera.m_focusPoint, camera.m_upDirection);
}

void DX12RenderCommandFactory::BuildRenderCommands_CalculateEntityMatrices(const Entity& entity, matrix& _outScaleMatrix, matrix& _outRotationMatrix, matrix& _outTranslationMatrix)
{
	const TransformComponent& transformComponent = *entity.m_components.at(ComponentType::TransformComponent)->As<TransformComponent>();

	_outScaleMatrix = ScaleMatrix(transformComponent.m_scale.x, transformComponent.m_scale.y, transformComponent.m_scale.z);
	_outRotationMatrix = RotationMatrix(ConvertToRadians(transformComponent.m_rotation.y), ConvertToRadians(transformComponent.m_rotation.x), ConvertToRadians(transformComponent.m_rotation.z));
	_outTranslationMatrix = TranslationMatrix(transformComponent.m_position.x, transformComponent.m_position.y, transformComponent.m_position.z);
}

void DX12RenderCommandFactory::BuildRenderCommands_Main(const Entity& entity, const matrix& camera, const matrix& ortho, const matrix& scale, const matrix& rotation,
	const matrix& translation, std::vector<std::unique_ptr<IRenderCommand>>& outRenderCommands)
{
	const DX12RenderComponent& renderableComponent = *entity.m_components.at(ComponentType::DX12RenderComponent)->As<DX12RenderComponent>();
	if (renderableComponent.m_pipelineState)
	{
		const matrix modelMatrix = scale * rotation * translation;
		const matrix mvpMatrix = modelMatrix * camera * ortho;

		outRenderCommands.push_back(std::make_unique<SetupSimplePixelShaderCommand>(renderableComponent.m_pipelineState, renderableComponent.m_rootSignature, renderableComponent.m_texture, mvpMatrix));
		outRenderCommands.push_back(std::make_unique<DrawMeshCommand>(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, renderableComponent.m_vertexBuffer));
	}
}

void DX12RenderCommandFactory::BuildRenderCommands_Debug(const Entity& entity, const matrix& camera, const matrix& ortho, const matrix& translation, std::vector<std::unique_ptr<IRenderCommand>>& outRenderCommands)
{
	const DebugRenderComponent& debugRenderComponent = *entity.m_components.at(ComponentType::DebugRenderComponent)->As<DebugRenderComponent>();
	if (debugRenderComponent.m_isEnabled)
	{
		const DX12DebugRenderingSettings& debugRenderingSettings = ((win::DX12Driver*)IRenderDriver::GetInstance())->GetDebugRenderingSettings();

		const AABB aabb = debugRenderComponent.m_debugAABB;
		const float width = abs(aabb.max.x - aabb.min.x);
		const float height = abs(aabb.max.y - aabb.min.y);
		const matrix debugScaleMatrix = ScaleMatrix(width, height, 1.f);
		const matrix debugModelMatrix = debugScaleMatrix * translation;
		const matrix debugMvpMatrix = debugModelMatrix * camera * ortho;

		outRenderCommands.push_back(std::make_unique<SetupAABBPixelShaderCommand>(debugRenderingSettings.GetPipelineState(), debugRenderingSettings.GetRootSignature(), debugMvpMatrix));
		outRenderCommands.push_back(std::make_unique<DrawMeshCommand>(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, debugRenderingSettings.GetVertexBuffer()));
	}
}

}
#endif
}
