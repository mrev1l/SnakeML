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
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/PhysicsComponent.h" // TODO ???
#include "system/ecs/components/MaterialComponent.h" // ???
#include "system/ecs/ECSManager.h"
#include "system/ecs/Entity.h"

#include "system/drivers/win/dx/pipeline/DX12CommandList.h"
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
	math::matrix cameraMatrix = math::LookAtMatrixLH(camera.m_eyePosition, camera.m_focusPoint, camera.m_upDirection);

	const DX12RenderComponent& renderableComponent = *(DX12RenderComponent*)entity.m_components.at(ComponentType::DX12RenderComponent);
	/*const*/ TransformComponent& transformComponent = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);
	/*const*/ PhysicsComponent& physicsComponent = *(PhysicsComponent*)entity.m_components.at(ComponentType::PhysicsComponent);

	//test TODO
	{
		//transformComponent.m_rotation += math::vector::forward * 0.3f;
		//physicsComponent.m_rotation = transformComponent.m_rotation;
	}

	math::matrix projection, orthogonal;
	const win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
	dx12Driver->GetMatrices(projection, orthogonal);

	const math::matrix scaleMatrix = math::ScaleMatrix(transformComponent.m_scale.x, transformComponent.m_scale.y, transformComponent.m_scale.z);
	const math::matrix rotationMatrix = math::RotationMatrix(math::ConvertToRadians(transformComponent.m_rotation.y), math::ConvertToRadians(transformComponent.m_rotation.x), math::ConvertToRadians(transformComponent.m_rotation.z));
	const math::matrix translationMatrix = math::TranslationMatrix(transformComponent.m_position.x, transformComponent.m_position.y, transformComponent.m_position.z);

	const math::matrix modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	if (renderableComponent.m_pipelineState)
	{
		const math::matrix mvpMatrix = modelMatrix * cameraMatrix * orthogonal;

		outRenderCommands.push_back(std::make_unique<SetupSimplePixelShaderCommand>(renderableComponent.m_pipelineState, renderableComponent.m_rootSignature, renderableComponent.m_texture, mvpMatrix));
		outRenderCommands.push_back(std::make_unique<DrawMeshCommand>(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, renderableComponent.m_vertexBuffer));
	}

	if (renderableComponent.m_debugPipelineState)
	{
		types::AABB aabb = physicsComponent.m_aabb;
		float width = abs(aabb.max.x - aabb.min.x);
		float height = abs(aabb.max.y - aabb.min.y);
		const math::matrix debugScaleMatrix = math::ScaleMatrix(width, height, 1.f);
		const math::matrix debugModelMatrix = debugScaleMatrix * translationMatrix;
		const math::matrix debugMvpMatrix = debugModelMatrix * cameraMatrix * orthogonal;

		outRenderCommands.push_back(std::make_unique<SetupAABBPixelShaderCommand>(renderableComponent.m_debugPipelineState, renderableComponent.m_debugRootSignature, debugMvpMatrix));
		outRenderCommands.push_back(std::make_unique<DrawMeshCommand>(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, renderableComponent.m_debugVertexBuffer));
	}
}

}
#endif
}
}