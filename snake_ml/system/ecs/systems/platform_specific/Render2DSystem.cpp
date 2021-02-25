// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "Render2DSystem.h"

#include "system/Application.h"
#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/DX12CommandList.h"
#include "system/drivers/win/dx/DX12RenderTarget.h"
#include "system/ecs/components/TransformComponent.h"
#include "system/ecs/components/platform_specific/DX12RenderComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{
namespace system
{

void Render2DSystem::Execute()
{
	win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
	const std::vector<Entity>& entities = ECSManager::GetInstance()->GetEntities();
	for (const Entity& entity : entities)
	{
		const DX12RenderComponent& renderable = *(DX12RenderComponent*)entity.m_components.at(ComponentType::DX12RenderComponent);
		const TransformComponent& transform = *(TransformComponent*)entity.m_components.at(ComponentType::TransformComponent);

		auto commandQueue = dx12Driver->GetDX12CommandQueue(win::DX12Driver::CommandQueueType::Direct);
		if (!commandQueue)
		{
			return;
		}
		auto commandList = commandQueue->GetCommandList();

		// Clear the render targets.
		{
			FLOAT clearColor[] = { 0.f, 0.f, 0.f, 1.0f };

			commandList->ClearTexture(dx12Driver->m_renderTarget.GetTexture(win::AttachmentPoint::Color0), clearColor);
			commandList->ClearDepthStencilTexture(dx12Driver->m_renderTarget.GetTexture(win::AttachmentPoint::DepthStencil), D3D12_CLEAR_FLAG_DEPTH);
		}

		commandList->SetPipelineState(renderable.m_pipelineState);
		commandList->SetGraphicsRootSignature(renderable.m_rootSignature);

		commandList->SetViewport(dx12Driver->m_viewport);
		commandList->SetScissorRect(dx12Driver->m_scissorRect);

		commandList->SetRenderTarget(dx12Driver->m_renderTarget);

		auto scaleMatrix = DirectX::XMMatrixScaling(transform.m_scale.m_x, transform.m_scale.m_y, transform.m_scale.m_z);
		auto rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(transform.m_rotation.m_x), DirectX::XMConvertToRadians(transform.m_rotation.m_y), DirectX::XMConvertToRadians(transform.m_rotation.m_z));
		auto translationMatrix = DirectX::XMMatrixTranslation(transform.m_position.m_x, transform.m_position.m_y, transform.m_position.m_z);

		const DirectX::XMMATRIX modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

		DirectX::XMMATRIX mvpMatrix = modelMatrix * dx12Driver->m_viewMatrix * dx12Driver->m_orthogonalMatrix;

		commandList->SetGraphics32BitConstants(win::RootParameters::MatricesCB, mvpMatrix);

		commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetVertexBuffer(0, renderable.m_vertexBuffer);
		
		commandList->Draw(renderable.m_vertexBuffer.GetNumVertices());

		commandQueue->ExecuteCommandList(commandList);

		// Present
		{
			auto presentCommandList = commandQueue->GetCommandList();
			auto& texture = dx12Driver->m_renderTarget.GetTexture(win::AttachmentPoint::Color0);
			auto& backBuffer = dx12Driver->m_backBufferTextures[dx12Driver->m_currentBackBufferIndex];

			if (texture.IsValid())
			{
				if (texture.GetD3D12ResourceDesc().SampleDesc.Count > 1)
				{
					presentCommandList->ResolveSubresource(backBuffer, texture);
				}
				else
				{
					presentCommandList->CopyResource(backBuffer, texture);
				}
			}

			win::DX12RenderTarget renderTarget;
			renderTarget.AttachTexture(win::AttachmentPoint::Color0, backBuffer);

			presentCommandList->TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
			commandQueue->ExecuteCommandList(presentCommandList);

			UINT syncInterval = dx12Driver->m_isVSync ? 1 : 0;
			UINT presentFlags = dx12Driver->m_isTearingSupported && !dx12Driver->m_isVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
			dxutils::ThrowIfFailed(dx12Driver->m_swapChain->Present(syncInterval, presentFlags));

			dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex] = commandQueue->Signal();
			dx12Driver->m_frameValues[dx12Driver->m_currentBackBufferIndex] = Application::s_frameCounter;

			dx12Driver->m_currentBackBufferIndex = dx12Driver->m_swapChain->GetCurrentBackBufferIndex();

			commandQueue->WaitForFenceValue(dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex]);

			dx12Driver->ReleaseStaleDescriptors(dx12Driver->m_frameValues[dx12Driver->m_currentBackBufferIndex]);
		}

		/// OLD
		/*
		auto backBuffer = dx12Driver->m_backBuffers[dx12Driver->m_currentBackBufferIndex];
		auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(dx12Driver->m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), dx12Driver->m_currentBackBufferIndex, dx12Driver->m_RTVDescriptorSize);
		auto dsv = dx12Driver->m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

		// Clear the render targets.
		{
			dxutils::TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			dxutils::ClearRTV(commandList, rtv, color);
			dxutils::ClearDSV(commandList, dsv);
		}

		commandList->SetPipelineState(renderable.m_pipelineState.Get());
		commandList->SetGraphicsRootSignature(renderable.m_rootSignature.Get());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->IASetVertexBuffers(0, 1, &renderable.m_vertexBufferView);
		commandList->IASetIndexBuffer(&renderable.m_indexBufferView);

		commandList->RSSetViewports(1, &dx12Driver->m_viewport);
		commandList->RSSetScissorRects(1, &dx12Driver->m_scissorRect);

		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

		auto scaleMatrix = DirectX::XMMatrixScaling(transform.m_scale.m_x, transform.m_scale.m_y, transform.m_scale.m_z);
		auto rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(transform.m_rotation.m_x), DirectX::XMConvertToRadians(transform.m_rotation.m_y), DirectX::XMConvertToRadians(transform.m_rotation.m_z));
		auto translationMatrix = DirectX::XMMatrixTranslation(transform.m_position.m_x, transform.m_position.m_y, transform.m_position.m_z);
		
		const DirectX::XMMATRIX modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;
		
		DirectX::XMMATRIX mvpMatrix = modelMatrix * dx12Driver->m_viewMatrix * dx12Driver->m_orthogonalMatrix;

		commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / _countof(DirectX::XMMATRIX::r), &mvpMatrix, 0);
		UINT vertexCount = renderable.m_vertexBufferView.SizeInBytes / renderable.m_vertexBufferView.StrideInBytes;
		commandList->DrawInstanced(vertexCount, 1, 0, 0);
		

		// Present
		{
			dxutils::TransitionResource(commandList, backBuffer,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);

			{
				UINT syncInterval = dx12Driver->m_isVSync ? 1 : 0;
				UINT presentFlags = dx12Driver->m_isTearingSupported && !dx12Driver->m_isVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
				dxutils::ThrowIfFailed(dx12Driver->m_swapChain->Present(syncInterval, presentFlags));
				dx12Driver->m_currentBackBufferIndex = dx12Driver->m_swapChain->GetCurrentBackBufferIndex();
			}

			commandQueue->WaitForFenceValue(dx12Driver->m_frameFenceValues[dx12Driver->m_currentBackBufferIndex]);
		}*/
	}
}

}
}