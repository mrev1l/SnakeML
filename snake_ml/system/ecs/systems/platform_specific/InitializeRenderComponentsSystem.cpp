// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeRenderComponentsSystem.h"

#include "system/drivers/win/dx/DX12Driver.h"
#include "system/ecs/components/platform_specific/DX12MaterialComponent.h"
#include "system/ecs/components/platform_specific/DX12RenderComponent.h"
#include "system/ecs/ECSManager.h"

namespace snakeml
{
namespace system
{

void InitializeRenderComponentsSystem::Execute()
{
	Iterator* materialComponents = ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::DX12MaterialComponent);
	DX12MaterialComponent* materials = (DX12MaterialComponent*)materialComponents->GetData();

	DX12RenderComponentIterator* renderComponentsIt = (DX12RenderComponentIterator*)IComponent::CreateIterator(ComponentType::DX12RenderComponent, materialComponents->Num());
	ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::DX12RenderComponent, renderComponentsIt);
	DX12RenderComponent* renderComponents = (DX12RenderComponent*)renderComponentsIt->GetData();

	win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();
	auto commandQueue = dx12Driver->GetDX12CommandQueue(win::DX12Driver::CommandQueueType::Copy);
	auto commandList = commandQueue->GetD3D12CommandList();

	for (int i = 0; i < materialComponents->Num(); ++i)
	{
		DX12RenderComponent& renderComponent = renderComponents[i];
		const DX12MaterialComponent& materialComponent = materials[i];

		renderComponent.m_entityId = materialComponent.m_entityId;
		// Upload vertex buffer data.
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateVertexBuffer;
		dxutils::UpdateBufferResource(device,
			commandList,
			&renderComponent.m_vertexBuffer, &intermediateVertexBuffer,
			materialComponent.m_vertices.size(), sizeof(materialComponent.m_vertices[0]), materialComponent.m_vertices.data());

		// Create the vertex buffer view.
		renderComponent.m_vertexBufferView.BufferLocation = renderComponent.m_vertexBuffer->GetGPUVirtualAddress();
		renderComponent.m_vertexBufferView.SizeInBytes = sizeof(materialComponent.m_vertices[0]) * materialComponent.m_vertices.size();
		renderComponent.m_vertexBufferView.StrideInBytes = sizeof(materialComponent.m_vertices[0]);

		// Upload index buffer data.
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateIndexBuffer;
		dxutils::UpdateBufferResource(device,
			commandList,
			&renderComponent.m_indexBuffer, &intermediateIndexBuffer,
			materialComponent.m_indices.size(), sizeof(materialComponent.m_indices[0]), materialComponent.m_indices.data());

		// Create index buffer view.
		renderComponent.m_indexBufferView.BufferLocation = renderComponent.m_indexBuffer->GetGPUVirtualAddress();
		renderComponent.m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		renderComponent.m_indexBufferView.SizeInBytes = sizeof(materialComponent.m_indices[0]) * materialComponent.m_indices.size();

		// Load the vertex shader.
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
		dxutils::ThrowIfFailed(D3DReadFileToBlob(materialComponent.m_vs.data(), &vertexShaderBlob));
		// Load the pixel shader.
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
		dxutils::ThrowIfFailed(D3DReadFileToBlob(materialComponent.m_ps.data(), &pixelShaderBlob));

		// Create a root signature.
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// Allow input layout and deny unnecessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		// A single 32-bit constant root parameter that is used by the vertex shader.
		CD3DX12_ROOT_PARAMETER1 rootParameters[1] = { };
		rootParameters[0].InitAsConstants(
			materialComponent.m_vsParamLayout.num32BitValues,
			materialComponent.m_vsParamLayout.shaderRegister,
			materialComponent.m_vsParamLayout.registerSpace,
			materialComponent.m_vsParamLayout.visibility);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
		// Serialize the root signature.
		Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		dxutils::ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
			featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

		// Create the root signature.
		dxutils::ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&renderComponent.m_rootSignature)));

		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
			CD3DX12_PIPELINE_STATE_STREAM_VS VS;
			CD3DX12_PIPELINE_STATE_STREAM_PS PS;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		} pipelineStateStream;

		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets = 1;
		rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		pipelineStateStream.pRootSignature = renderComponent.m_rootSignature.Get();
		pipelineStateStream.InputLayout = { materialComponent.m_vsInputLayout.data(), (UINT)materialComponent.m_vsInputLayout.size() };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
		pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
		pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineStateStream.RTVFormats = rtvFormats;

		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
		};
		dxutils::ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&renderComponent.m_pipelineState)));

		auto fenceValue = commandQueue->ExecuteCommandList(commandList);
		commandQueue->WaitForFenceValue(fenceValue);
	}
}

}
}
