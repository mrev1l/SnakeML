// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12DebugRenderingSettings.h"

#include "system/drivers/win/dx/helpers/directX_utils.h"
#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/pipeline/DX12CommandList.h"

namespace snakeml
{

#ifdef _WINDOWS
namespace win
{
std::vector<std::pair<float3, float3>> DX12DebugRenderingSettings::s_debugAABBVertices
{
	{ { -0.5f, +0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { +0.5f, -0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { -0.5f, -0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { -0.5f, +0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { +0.5f, +0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { +0.5f, -0.5f, 0.f }, { 1.f, 1.f, 1.f } },
};

D3D12_INPUT_ELEMENT_DESC DX12DebugRenderingSettings::s_aabbInputLayoutDefinition[2] =
{
	{
		"POSITION",
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,
		0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
	},
	{
		"COLOR",
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,
		0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
		}
};

void DX12DebugRenderingSettings::Init()
{
	Init_AABBSettings();
}

void DX12DebugRenderingSettings::Init_AABBSettings()
{
	DX12Driver* dx12Driver = (DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();
	auto commandQueue = dx12Driver->GetDX12CommandQueue(DX12Driver::CommandQueueType::Copy);
	auto commandList = commandQueue ? commandQueue->GetCommandList() : nullptr;

	Init_AABBVertexBuffer(commandList);
	Init_AABBRootSignature(device);
	Init_PipelineState(device, dx12Driver->GetMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT));

	if (commandQueue)
	{
		auto fenceValue = commandQueue->ExecuteCommandList(commandList);
		commandQueue->WaitForFenceValue(fenceValue);
	}
}

void DX12DebugRenderingSettings::Init_AABBVertexBuffer(std::shared_ptr<DX12CommandList> commandList)
{
	commandList->CopyVertexBuffer(m_debugAABBVertexBuffer, s_debugAABBVertices);
}

void DX12DebugRenderingSettings::Init_AABBRootSignature(Microsoft::WRL::ComPtr<ID3D12Device2> device)
{
	std::vector<CD3DX12_ROOT_PARAMETER1> aabbRootParameters = CreateAABBRootParameters();
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = CreateFeatureData(device);

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootAABBSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC aabbRootSignatureDescription;
	aabbRootSignatureDescription.Init_1_1(aabbRootParameters.size(), aabbRootParameters.data(), 0, nullptr, rootAABBSignatureFlags);

	m_debugAABBRootSignature.SetRootSignatureDesc(aabbRootSignatureDescription.Desc_1_1, featureData.HighestVersion);
}

void DX12DebugRenderingSettings::Init_PipelineState(Microsoft::WRL::ComPtr<ID3D12Device2> device, DXGI_SAMPLE_DESC sampleDesc)
{
	CD3DX12_RASTERIZER_DESC rasterizerDescDebug(D3D12_DEFAULT);
	rasterizerDescDebug.FillMode = D3D12_FILL_MODE_WIREFRAME;
	const D3D12_RT_FORMAT_ARRAY rtvFormats = { {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB}, 1 };
	CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT dx12Layout = CreateAABBInputLayout();

	Microsoft::WRL::ComPtr<ID3DBlob> debugVertexShaderBlob, debugPixelShaderBlob;
	DX12Utils::ThrowIfFailed(D3DReadFileToBlob(s_debugVSPath, &debugVertexShaderBlob));
	DX12Utils::ThrowIfFailed(D3DReadFileToBlob(s_debugPSPath, &debugPixelShaderBlob));

	PipelineStateStream pipelineStateStream = {
		m_debugAABBRootSignature.GetRootSignature().Get(),
		dx12Layout,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		CD3DX12_SHADER_BYTECODE(debugVertexShaderBlob.Get()),
		CD3DX12_SHADER_BYTECODE(debugPixelShaderBlob.Get()),
		DXGI_FORMAT_D32_FLOAT,
		rtvFormats,
		sampleDesc,
		rasterizerDescDebug };

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};
	DX12Utils::ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_debugAABBPipelineState)));
}

std::vector<CD3DX12_ROOT_PARAMETER1> DX12DebugRenderingSettings::CreateAABBRootParameters()
{
	std::vector<CD3DX12_ROOT_PARAMETER1> aabbRootParams(1);
	aabbRootParams[0].InitAsConstants(s_modelMatrixNumValues, 0, 0, D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX);
	return aabbRootParams;
}

CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT DX12DebugRenderingSettings::CreateAABBInputLayout()
{
	CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT result;
	result = { s_aabbInputLayoutDefinition, sizeof(s_aabbInputLayoutDefinition) / sizeof(s_aabbInputLayoutDefinition[0]) };
	return result;
}

D3D12_FEATURE_DATA_ROOT_SIGNATURE DX12DebugRenderingSettings::CreateFeatureData(Microsoft::WRL::ComPtr<ID3D12Device2> device)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
	return featureData;
}

}
#endif

}