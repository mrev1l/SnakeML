// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "InitializeRenderComponentsSystem.h"

#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/pipeline/DX12CommandList.h"

#include "system/drivers/win/os/helpers/win_utils.h"

#include "system/ecs/ECSManager.h"
#include "system/ecs/components/platform_specific/win/DX12RenderComponent.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

void InitializeRenderComponentsSystem::Execute()
{
	MaterialComponentIterator* materialsIt = ECSManager::GetInstance()->GetComponents<MaterialComponentIterator>();
	MeshComponentIterator* meshesIt = ECSManager::GetInstance()->GetComponents<MeshComponentIterator>();

	//DX12RenderComponentIterator* renderComponentsIt = (DX12RenderComponentIterator*)IComponent::CreateIterator(ComponentType::DX12RenderComponent, materialsIt->Size());
	DX12RenderComponentIterator* renderComponentsIt = ECSManager::GetInstance()->GetComponents<DX12RenderComponentIterator>();

	DX12Driver* dx12Driver = (DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();
	auto commandQueue = dx12Driver->GetDX12CommandQueue(DX12Driver::CommandQueueType::Copy);
	auto commandList = commandQueue ? commandQueue->GetCommandList() : nullptr;

	if (m_targetEntities.empty())
	{
		for (size_t i = 0; i < materialsIt->Size(); ++i)
		{
			const MaterialComponent& materialComponent = materialsIt->At(i);

			Entity& entity = ECSManager::GetInstance()->GetEntity(materialComponent.m_entityId);

			ASSERT(entity.m_components.contains(ComponentType::MeshComponent), "[InitializeRenderComponentsSystem::Execute] : Renderables initialization is going to fail");
			const MeshComponent& mesh = *entity.m_components.at(ComponentType::MeshComponent)->As<MeshComponent>();

			DX12RenderComponent& renderComponent = renderComponentsIt->Add();
			InitRenderComponent(commandList, materialComponent, mesh, renderComponent);

			entity.m_components.insert({ ComponentType::DX12RenderComponent, &renderComponent });
		}
	}
	else
	{
		for (uint32_t entityId : m_targetEntities)
		{
			Entity& entity = ECSManager::GetInstance()->GetEntity(entityId);
			const MaterialComponent& materialComponent = *entity.m_components.at(ComponentType::MaterialComponent)->As<MaterialComponent>();
			const MeshComponent& mesh = *entity.m_components.at(ComponentType::MeshComponent)->As<MeshComponent>();

			DX12RenderComponent& renderComponent = renderComponentsIt->Add();
			InitRenderComponent(commandList, materialComponent, mesh, renderComponent);

			entity.m_components.insert({ ComponentType::DX12RenderComponent, &renderComponent });
		}
	}

	if (commandQueue)
	{
		auto fenceValue = commandQueue->ExecuteCommandList(commandList);
		commandQueue->WaitForFenceValue(fenceValue);
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent(std::shared_ptr<DX12CommandList> commandList, const MaterialComponent& materialComponent, const MeshComponent& meshComponent, DX12RenderComponent& _outRenderComponent)
{
	_outRenderComponent.m_entityId = materialComponent.m_entityId;

	if (materialComponent.m_texturePaths.size() == 1) // TODO : revise
	{
		InitRenderComponent_LoadTextures(commandList, materialComponent.m_texturePaths[0], _outRenderComponent.m_texture);
	}
	else
	{
		InitRenderComponent_LoadTextures(commandList, materialComponent.m_texturePaths, _outRenderComponent.m_texture);
	}

	const std::vector<std::pair<float3, float2>>& vertices = materialComponent.m_vs.empty() ? std::vector<std::pair<float3, float2>>() : meshComponent.m_vertices;
	InitRenderComponent_LoadBuffers(commandList, vertices, _outRenderComponent.m_vertexBuffer);
	if (materialComponent.m_entityId == 0)
	{
		DX12Driver* dx12Driver = (DX12Driver*)IRenderDriver::GetInstance();
		//InitRenderComponent_LoadBuffers(commandList, vertices, dx12Driver->testVertexBuffer);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, pixelShaderBlob;
	InitRenderComponent_LoadShaders(materialComponent.m_vs.data(), materialComponent.m_ps.data(), vertexShaderBlob, pixelShaderBlob);

	std::vector<RootParameters> rootParamsIds;
	if (vertexShaderBlob.Get() && pixelShaderBlob.Get())
	{
		rootParamsIds = { RootParameters::MatricesCB, RootParameters::Textures };
	}
	if (materialComponent.m_texturePaths.size() > 1)
	{
		rootParamsIds.push_back(RootParameters::TextureId);
	}
	InitRenderComponent_InitializeRootSignatures(rootParamsIds, _outRenderComponent.m_rootSignature);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	InitRenderComponent_GenerateInputLayouts(materialComponent.m_inputLayoutEntries, inputLayout);

	InitRenderComponent_InitializePSOs(
		_outRenderComponent.m_rootSignature.GetRootSignature(),
		inputLayout,
		vertexShaderBlob,
		pixelShaderBlob,
		_outRenderComponent.m_pipelineState);
}

void InitializeRenderComponentsSystem::InitRenderComponent_LoadTextures(std::shared_ptr<DX12CommandList> commandList, std::wstring texturePath, DX12Texture& _outTexture)
{
	if (!texturePath.empty())
	{
		commandList->LoadTexture(_outTexture, { texturePath });
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent_LoadTextures(std::shared_ptr<DX12CommandList> commandList, std::vector<std::wstring> texturesPath, DX12Texture& _outTexture)
{
	if (!texturesPath.empty())
	{
		commandList->LoadTexture(_outTexture, texturesPath);
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent_LoadBuffers(
	std::shared_ptr<DX12CommandList> commandList,
	const std::vector<std::pair<float3, float2>>& geometryVertices, 
	DX12VertexBuffer& _outGeometryVB)
{
	if (!geometryVertices.empty())
	{
		commandList->CopyVertexBuffer(_outGeometryVB, geometryVertices);
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent_LoadShaders(
	const std::wstring& vsPath,
	const std::wstring& psPath,
	Microsoft::WRL::ComPtr<ID3DBlob>& _outVsBlob, 
	Microsoft::WRL::ComPtr<ID3DBlob>& _outPsBlob)
{
	if(!vsPath.empty())
	{
		WinUtils::ThrowIfFailed(D3DReadFileToBlob(vsPath.c_str(), &_outVsBlob));
	}
	
	if (!psPath.empty())
	{
		WinUtils::ThrowIfFailed(D3DReadFileToBlob(psPath.c_str(), &_outPsBlob));
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent_InitializeRootSignatures(
	const std::vector<RootParameters>& rootParamsIds,
	DX12RootSignature& _outRootSignature)
{
	if(!rootParamsIds.empty())
	{
		const CD3DX12_DESCRIPTOR_RANGE1 descriptorRage(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters = CreateRootParameters(rootParamsIds, &descriptorRage);
		CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
		CreateRootSignature(rootParameters, 1u, &linearRepeatSampler, _outRootSignature);
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent_GenerateInputLayouts(
	const std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries,
	std::vector<D3D12_INPUT_ELEMENT_DESC>& _outInputLayout)
{
	if (!inputLayoutEntries.empty())
	{
		GenerateInputLayout(inputLayoutEntries, _outInputLayout);
	}
}

void InitializeRenderComponentsSystem::InitRenderComponent_InitializePSOs(
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob,
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob,
	Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outPipelineState)
{
	if (rootSignature.Get())
	{
		CreatePipelineState(
			rootSignature,
			inputLayout,
			vsBlob,
			psBlob,
			CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
			_outPipelineState);
	}
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
	rootSignatureDescription.Init_1_1(static_cast<UINT>(rootParameters.size()), rootParameters.data(), numStaticSamplers, pStaticSamplers, rootSignatureFlags);
	
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
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilDesc;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendDesc;
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

	const DX12GlobalRenderSettings& globalRenderSettings = dx12Driver->GetGlobalRenderSettings();

	pipelineStateStream.BlendDesc = globalRenderSettings.GetAlphaBlendDesc();
	pipelineStateStream.DepthStencilDesc = globalRenderSettings.GetNoDepthStencilDesc();;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};
	WinUtils::ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&_outPipelineState)));
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
	ASSERT(false, "InitializeRenderComponentsGetInputLayoutFormat Incorrect input param");
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
	ASSERT(false, "InitializeRenderComponentsGetShaderSemanticName Incorrect input param");
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

std::vector<CD3DX12_ROOT_PARAMETER1> InitializeRenderComponentsSystem::CreateRootParameters(const std::vector<RootParameters>& rootParamsIds, const D3D12_DESCRIPTOR_RANGE1* descriptorRange)
{
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(rootParamsIds.size());
	for (size_t i = 0u; i < rootParameters.size(); ++i)
	{
		switch (rootParamsIds[i])
		{
		case RootParameters::MatricesCB:
		{
			rootParameters[i].InitAsConstants(
				GetRootParameterNumValues(RootParameters::MatricesCB),
				0,
				0,
				GetRootParameterShaderVisibility(RootParameters::MatricesCB));
		}
		break;
		case RootParameters::Textures:
		{
			rootParameters[i].InitAsDescriptorTable(
				GetRootParameterNumValues(RootParameters::Textures),
				descriptorRange,
				GetRootParameterShaderVisibility(RootParameters::Textures));
		}
		break;
		case RootParameters::TextureId:
		{
			rootParameters[i].InitAsConstants(
				GetRootParameterNumValues(RootParameters::TextureId),
				1,
				0,
				GetRootParameterShaderVisibility(RootParameters::TextureId));
		}
		break;
		default:
			ASSERT(false, "Missed enum entry");
			break;
		}
	}
	return rootParameters;
}

UINT InitializeRenderComponentsSystem::GetRootParameterNumValues(RootParameters paramType)
{
	constexpr UINT k_mvpNumValues = 16u;
	switch (paramType)
	{
	case MatricesCB:		return k_mvpNumValues;
	case TextureId:
	case Textures:			return 1u;
	default:
		static_assert(RootParameters::NumRootParameters == 3u);
	}
	ASSERT(false, "InitializeRenderComponentsGetRootParameterNumValues Incorrect input param");
	return 0u;
}

D3D12_SHADER_VISIBILITY InitializeRenderComponentsSystem::GetRootParameterShaderVisibility(RootParameters paramType)
{
	switch (paramType)
	{
	case MatricesCB:		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
	case TextureId:
	case Textures:			return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;
	default:
		static_assert(RootParameters::NumRootParameters == 3u);
	}
	ASSERT(false, "InitializeRenderComponentsGetRootParameterShaderVisibility Incorrect input param");
	return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
}

}
#endif
}
