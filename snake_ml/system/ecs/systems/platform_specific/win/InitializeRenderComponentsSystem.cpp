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

std::vector<std::pair<types::vec3<float>, types::vec3<float>>> InitializeRenderComponentsSystem::s_debugAABBVertices
{
	{ { -0.5f, +0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { +0.5f, -0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { -0.5f, -0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { -0.5f, +0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { +0.5f, +0.5f, 0.f }, { 1.f, 1.f, 1.f } },
	{ { +0.5f, -0.5f, 0.f }, { 1.f, 1.f, 1.f } },
};

std::vector<MaterialComponent::InputLayoutEntries> InitializeRenderComponentsSystem::s_debugInputLayoutEntries
{
	MaterialComponent::InputLayoutEntries::Position, 
	MaterialComponent::InputLayoutEntries::Color
};

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

		InitRenderComponent(commandList, materialComponent, renderComponent);

		if (commandQueue)
		{
			auto fenceValue = commandQueue->ExecuteCommandList(commandList);
			commandQueue->WaitForFenceValue(fenceValue);
		}
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent(std::shared_ptr<DX12CommandList> commandList, const MaterialComponent& materialComponent, DX12RenderComponent& _outRenderComponent)
{
	_outRenderComponent.m_entityId = materialComponent.m_entityId;

	InitRenderComponent_LoadTextures(commandList, materialComponent.m_texturePath, _outRenderComponent.m_texture);

	InitRenderComponent_LoadBuffers(commandList, materialComponent.m_vertices, s_debugAABBVertices, _outRenderComponent.m_vertexBuffer, _outRenderComponent.m_debugVertexBuffer);

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, pixelShaderBlob, debugVertexShaderBlob, debugPixelShaderBlob;
	InitRenderComponent_LoadShaders(materialComponent.m_vs.data(), materialComponent.m_ps.data(), s_debugVSPath, s_debugPSPath,
		vertexShaderBlob, pixelShaderBlob, debugVertexShaderBlob, debugPixelShaderBlob);

	InitRenderComponent_InitializeRootSignatures(_outRenderComponent.m_rootSignature, _outRenderComponent.m_debugRootSignature);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout, inputLayoutDebug;
	InitRenderComponent_GenerateInputLayouts(materialComponent.m_inputLayoutEntries, s_debugInputLayoutEntries, inputLayout, inputLayoutDebug);

	InitRenderComponent_InitializePSOs(
		_outRenderComponent.m_rootSignature.GetRootSignature(),
		_outRenderComponent.m_debugRootSignature.GetRootSignature(),
		inputLayout,
		inputLayoutDebug,
		vertexShaderBlob,
		debugVertexShaderBlob,
		pixelShaderBlob,
		debugPixelShaderBlob,
		_outRenderComponent.m_pipelineState,
		_outRenderComponent.m_debugPipelineState);
}

void InitializeRenderComponentsSystem::InitRenderComponent_LoadTextures(std::shared_ptr<DX12CommandList> commandList, std::wstring texturePath, DX12Texture& _outTexture)
{
	commandList->LoadTextureFromFile(_outTexture, texturePath);
}

void InitializeRenderComponentsSystem::InitRenderComponent_LoadBuffers(
	std::shared_ptr<DX12CommandList> commandList,
	const std::vector<std::pair<types::vec3<float>, types::vec2<float>>>& geometryVertices, 
	const std::vector<std::pair<types::vec3<float>, types::vec3<float>>>& debugGeometryVertices, 
	DX12VertexBuffer& _outGeometryVB, 
	DX12VertexBuffer& _outDebugGeometryVB)
{
	commandList->CopyVertexBuffer(_outGeometryVB, geometryVertices);
	commandList->CopyVertexBuffer(_outDebugGeometryVB, debugGeometryVertices);
}

void InitializeRenderComponentsSystem::InitRenderComponent_LoadShaders(
	const wchar_t* vsPath, 
	const wchar_t* psPath, 
	const wchar_t* debugVsPath, 
	const wchar_t* debugPsPath, 
	Microsoft::WRL::ComPtr<ID3DBlob>& _outVsBlob, 
	Microsoft::WRL::ComPtr<ID3DBlob>& _outPsBlob, 
	Microsoft::WRL::ComPtr<ID3DBlob>& _outDebugVsBlob, 
	Microsoft::WRL::ComPtr<ID3DBlob>& _outDebugPsBlob)
{
	DX12Utils::ThrowIfFailed(D3DReadFileToBlob(vsPath, &_outVsBlob));
	DX12Utils::ThrowIfFailed(D3DReadFileToBlob(psPath, &_outPsBlob));
	DX12Utils::ThrowIfFailed(D3DReadFileToBlob(debugVsPath, &_outDebugVsBlob));
	DX12Utils::ThrowIfFailed(D3DReadFileToBlob(debugPsPath, &_outDebugPsBlob));
}

void InitializeRenderComponentsSystem::InitRenderComponent_InitializeRootSignatures(DX12RootSignature& _outRootSignature, DX12RootSignature& _outDebugRootSignature)
{
	{
		const CD3DX12_DESCRIPTOR_RANGE1 descriptorRage(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters = CreateRootParameters_Main(&descriptorRage);
		CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
		CreateRootSignature(rootParameters, 1u, &linearRepeatSampler, _outRootSignature);
	}

	{
		std::vector<CD3DX12_ROOT_PARAMETER1> rootParametersDebug = CreateRootParameters_Debug();
		CreateRootSignature(rootParametersDebug, 0, nullptr, _outDebugRootSignature);
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent_GenerateInputLayouts(
	const std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries,
	const std::vector<MaterialComponent::InputLayoutEntries>& debugInputLayoutEntries,
	std::vector<D3D12_INPUT_ELEMENT_DESC>& _outInputLayout,
	std::vector<D3D12_INPUT_ELEMENT_DESC>& _outDebugInputLayout)
{
	GenerateInputLayout(inputLayoutEntries, _outInputLayout);
	GenerateInputLayout(debugInputLayoutEntries, _outDebugInputLayout);
}

void InitializeRenderComponentsSystem::InitRenderComponent_InitializePSOs(
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
	Microsoft::WRL::ComPtr<ID3D12RootSignature> debugRootSignature,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& debugInputLayout,
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob,
	Microsoft::WRL::ComPtr<ID3DBlob> debugVsBlob,
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob,
	Microsoft::WRL::ComPtr<ID3DBlob> debugPsBlob,
	Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outPipelineState,
	Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outDebugPipelineState)
{
	CreatePipelineState(
		rootSignature,
		inputLayout,
		vsBlob,
		psBlob,
		CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
		_outPipelineState);

	CD3DX12_RASTERIZER_DESC rasterizerDescDebug(D3D12_DEFAULT);
	rasterizerDescDebug.FillMode = D3D12_FILL_MODE_WIREFRAME;
	CreatePipelineState(
		debugRootSignature,
		debugInputLayout,
		debugVsBlob,
		debugPsBlob,
		rasterizerDescDebug,
		_outDebugPipelineState);
}

void InitializeRenderComponentsSystem::CreateRootSignature(
	const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters,
	UINT numStaticSamplers,
	const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers,
	DX12RootSignature& _outRootSignature)
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

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(rootParameters.size(), rootParameters.data(), numStaticSamplers, pStaticSamplers, rootSignatureFlags);
	
	_outRootSignature.SetRootSignatureDesc(rootSignatureDescription.Desc_1_1, featureData.HighestVersion);
}

void InitializeRenderComponentsSystem::CreatePipelineState(
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob,
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob,
	CD3DX12_RASTERIZER_DESC rasterizerDesc,
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
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
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
	pipelineStateStream.Rasterizer = rasterizerDesc;

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
	case MaterialComponent::InputLayoutEntries::Color:		return DXGI_FORMAT_R32G32B32_FLOAT;
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

std::vector<CD3DX12_ROOT_PARAMETER1> InitializeRenderComponentsSystem::CreateRootParameters_Main(const D3D12_DESCRIPTOR_RANGE1* descriptorRange)
{
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(RootParameters::NumRootParameters);
	rootParameters[RootParameters::MatricesCB].InitAsConstants(
		GetRootParameterNumValues(RootParameters::MatricesCB),
		0,
		0,
		GetRootParameterShaderVisibility(RootParameters::MatricesCB)
	);
	rootParameters[RootParameters::Textures].InitAsDescriptorTable(
		GetRootParameterNumValues(RootParameters::Textures),
		descriptorRange,
		GetRootParameterShaderVisibility(RootParameters::Textures)
	);
	return rootParameters;
}

std::vector<CD3DX12_ROOT_PARAMETER1> InitializeRenderComponentsSystem::CreateRootParameters_Debug()
{
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(1);
	rootParameters[RootParameters::MatricesCB].InitAsConstants(
		GetRootParameterNumValues(RootParameters::MatricesCB), 
		0, 
		0, 
		GetRootParameterShaderVisibility(RootParameters::MatricesCB));

	return rootParameters;
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
