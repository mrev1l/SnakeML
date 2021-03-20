// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeRenderComponentsSystem.h"

#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/pipeline/DX12CommandList.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/components/platform_specific/win/DX12RenderComponent.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

void InitializeRenderComponentsSystem::Execute()
{
	Iterator* materialComponents = ECSManager::GetInstance()->GetComponentsPool().GetComponents(ComponentType::MaterialComponent);
	MaterialComponent* materials = (MaterialComponent*)materialComponents->GetData();

	DX12RenderComponentIterator* renderComponentsIt = (DX12RenderComponentIterator*)IComponent::CreateIterator(ComponentType::DX12RenderComponent, materialComponents->Num());
	ECSManager::GetInstance()->GetComponentsPool().InsertComponents(ComponentType::DX12RenderComponent, renderComponentsIt);
	DX12RenderComponent* renderComponents = (DX12RenderComponent*)renderComponentsIt->GetData();

	DX12Driver* dx12Driver = (DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();
	auto commandQueue = dx12Driver->GetDX12CommandQueue(DX12Driver::CommandQueueType::Copy);
	auto commandList = commandQueue ? commandQueue->GetCommandList() : nullptr;

	for (size_t i = 0; i < materialComponents->Num(); ++i)
	{
		DX12RenderComponent& renderComponent = renderComponents[i];
		const MaterialComponent& materialComponent = materials[i];

		renderComponent.m_entityId = materialComponent.m_entityId;

		commandList->LoadTextureFromFile(renderComponent.m_texture, materialComponent.m_texturePath);

		commandList->CopyVertexBuffer(renderComponent.m_vertexBuffer, materialComponent.m_vertices);

		// Load the vertex shader.
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
		DX12Utils::ThrowIfFailed(D3DReadFileToBlob(materialComponent.m_vs.data(), &vertexShaderBlob));
		// Load the pixel shader.
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
		DX12Utils::ThrowIfFailed(D3DReadFileToBlob(materialComponent.m_ps.data(), &pixelShaderBlob));

		CreateRootSignature(renderComponent.m_rootSignature);

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
		GenerateInputLayout(materialComponent.m_inputLayoutEntries, inputLayout);

		CreatePipelineState(
			renderComponent.m_rootSignature.GetRootSignature(),
			inputLayout,
			vertexShaderBlob,
			pixelShaderBlob,
			renderComponent.m_pipelineState);

		if (commandQueue)
		{
			auto fenceValue = commandQueue->ExecuteCommandList(commandList);
			commandQueue->WaitForFenceValue(fenceValue);
		}
	}
}

void InitializeRenderComponentsSystem::CreateRootSignature(DX12RootSignature& _outRootSignature)
{
	DX12Driver* dx12Driver = (DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();

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
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CD3DX12_DESCRIPTOR_RANGE1 descriptorRage(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters] = { };
	rootParameters[RootParameters::MatricesCB].InitAsConstants(
		GetRootParameterNumValues(RootParameters::MatricesCB),
		0,
		0,
		GetRootParameterShaderVisibility(RootParameters::MatricesCB)
	);
	rootParameters[RootParameters::Textures].InitAsDescriptorTable(
		GetRootParameterNumValues(RootParameters::Textures),
		&descriptorRage,
		GetRootParameterShaderVisibility(RootParameters::Textures)
	);

	CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1, &linearRepeatSampler, rootSignatureFlags);
	
	_outRootSignature.SetRootSignatureDesc(rootSignatureDescription.Desc_1_1, featureData.HighestVersion);
}

void InitializeRenderComponentsSystem::CreatePipelineState(
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob,
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob,
	Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outPipelineState)
{
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
	} pipelineStateStream;

	DX12Driver* dx12Driver = (DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();
	// Check the best multisample quality level that can be used for the given back buffer format.
	DXGI_SAMPLE_DESC sampleDesc = dx12Driver->GetMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT);

	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	pipelineStateStream.pRootSignature = rootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.SampleDesc = sampleDesc;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};
	DX12Utils::ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&_outPipelineState)));
}

DXGI_FORMAT InitializeRenderComponentsSystem::GetInputLayoutFormat(MaterialComponent::InputLayoutEntries layoutEntry)
{
	switch (layoutEntry)
	{
	case MaterialComponent::InputLayoutEntries::Position:	return DXGI_FORMAT_R32G32B32_FLOAT;
	case MaterialComponent::InputLayoutEntries::Color:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case MaterialComponent::InputLayoutEntries::UV:			return DXGI_FORMAT_R32G32_FLOAT;
	default:
		static_assert(static_cast<size_t>(MaterialComponent::InputLayoutEntries::Count) == 3u);
	}
	ASSERT(false, "InitializeRenderComponentsSystem::GetInputLayoutFormat Incorrect input param");
	return DXGI_FORMAT_UNKNOWN;
}

DX12Utils::DX12ShaderSemanticName InitializeRenderComponentsSystem::GetShaderSemanticName(MaterialComponent::InputLayoutEntries layoutEntry)
{
	switch (layoutEntry)
	{
	case MaterialComponent::InputLayoutEntries::Position:	return DX12Utils::DX12ShaderSemanticName::Position;
	case MaterialComponent::InputLayoutEntries::Color:		return DX12Utils::DX12ShaderSemanticName::Color;
	case MaterialComponent::InputLayoutEntries::UV:			return DX12Utils::DX12ShaderSemanticName::TexCoord;
	default:
		static_assert(static_cast<size_t>(MaterialComponent::InputLayoutEntries::Count) == 3u);
	}
	ASSERT(false, "InitializeRenderComponentsSystem::GetShaderSemanticName Incorrect input param");
	return DX12Utils::DX12ShaderSemanticName::Color;
}

void InitializeRenderComponentsSystem::GenerateInputLayout(const std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries, std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout)
{
	outInputLayout.resize(inputLayoutEntries.size());

	for (std::vector<D3D12_INPUT_ELEMENT_DESC>::size_type idx = 0; idx < outInputLayout.size(); ++idx)
	{
		D3D12_INPUT_ELEMENT_DESC& inputLayourEntry = outInputLayout[idx];

		inputLayourEntry = {
			DX12Utils::GetShaderSemanticNameStr(GetShaderSemanticName(inputLayoutEntries[idx])),
			0,
			GetInputLayoutFormat(inputLayoutEntries[idx]),
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		};
	}
}

UINT InitializeRenderComponentsSystem::GetRootParameterNumValues(RootParameters paramType)
{
	constexpr UINT k_mvpNumValues = 16u;
	switch (paramType)
	{
	case MatricesCB:		return k_mvpNumValues;
	case Textures:			return 1u;
	default:
		static_assert(RootParameters::NumRootParameters == 2u);
	}
	ASSERT(false, "InitializeRenderComponentsSystem::GetRootParameterNumValues Incorrect input param");
	return 0u;
}

D3D12_SHADER_VISIBILITY InitializeRenderComponentsSystem::GetRootParameterShaderVisibility(RootParameters paramType)
{
	switch (paramType)
	{
	case MatricesCB:		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
	case Textures:			return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;
	default:
		static_assert(RootParameters::NumRootParameters == 2u);
	}
	ASSERT(false, "InitializeRenderComponentsSystem::GetRootParameterShaderVisibility Incorrect input param");
	return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
}

}
#endif
}
}
