// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/drivers/win/dx/helpers/directX_utils.h"
#include "system/drivers/win/dx/pipeline/DX12RootSignature.h"

#include "system/ecs/ISystem.h"
#include "system/ecs/components/MaterialComponent.h"
#include "system/ecs/components/MeshComponent.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

class DX12CommandList;
class DX12RenderComponent;
class DX12Texture;
class DX12VertexBuffer;

// TODO REVIEW OPTIONAL CREATION
class InitializeRenderComponentsSystem : public ISystem
{
public:
	InitializeRenderComponentsSystem() : ISystem() {}

	void Execute() override;

private:
	static void InitRenderComponent(std::shared_ptr<DX12CommandList> commandList, const MaterialComponent& materialComponent, const MeshComponent& meshComponent, DX12RenderComponent& _outRenderComponent);
	static void InitRenderComponent_LoadTextures(std::shared_ptr<DX12CommandList> commandList, std::wstring texturePath, DX12Texture& _outTexture);
	static void InitRenderComponent_LoadBuffers(
		std::shared_ptr<DX12CommandList> commandList,
		const std::vector<std::pair<types::float3, types::float2>>& geometryVertices,
		const std::vector<std::pair<types::float3, types::float3>>& debugGeometryVertices,
		DX12VertexBuffer& _outGeometryVB,
		DX12VertexBuffer& _outDebugGeometryVB);
	static void InitRenderComponent_LoadShaders(
		const std::wstring& vsPath,
		const std::wstring& psPath,
		const std::wstring& debugVsPath,
		const std::wstring& debugPsPath,
		Microsoft::WRL::ComPtr<ID3DBlob>& _outVsBlob,
		Microsoft::WRL::ComPtr<ID3DBlob>& _outPsBlob,
		Microsoft::WRL::ComPtr<ID3DBlob>& _outDebugVsBlob,
		Microsoft::WRL::ComPtr<ID3DBlob>& _outDebugPsBlob);
	static void InitRenderComponent_InitializeRootSignatures(
		const std::vector<RootParameters>& rootParamsIds,
		const std::vector<RootParameters>& debugRootParamsIds,
		DX12RootSignature& _outRootSignature,
		DX12RootSignature& _outDebugRootSignature);
	static void InitRenderComponent_GenerateInputLayouts(
		const std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries,
		const std::vector<MaterialComponent::InputLayoutEntries>& debugInputLayoutEntries,
		std::vector<D3D12_INPUT_ELEMENT_DESC>& _outInputLayout,
		std::vector<D3D12_INPUT_ELEMENT_DESC>& _outDebugInputLayout);
	static void InitRenderComponent_InitializePSOs(
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
		Microsoft::WRL::ComPtr<ID3D12RootSignature> debugRootSignature,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& debugInputLayout,
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob,
		Microsoft::WRL::ComPtr<ID3DBlob> debugVsBlob,
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob,
		Microsoft::WRL::ComPtr<ID3DBlob> debugPsBlob,
		Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outPipelineState,
		Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outDebugPipelineState);
	
	static void CreateRootSignature(
		const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters,
		UINT numStaticSamplers,
		const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers,
		DX12RootSignature& _outRootSignature);
	static void CreatePipelineState(
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob,
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob,
		CD3DX12_RASTERIZER_DESC rasterizerDesc,
		Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outPipelineState);

	static DXGI_FORMAT GetInputLayoutFormat(MaterialComponent::InputLayoutEntries layoutEntry);
	static DX12Utils::DX12ShaderSemanticName GetShaderSemanticName(MaterialComponent::InputLayoutEntries layoutEntry);
	static void GenerateInputLayout(const std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries, std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout);
	
	static std::vector<CD3DX12_ROOT_PARAMETER1> CreateRootParameters(const std::vector<RootParameters>& rootParamsIds, const D3D12_DESCRIPTOR_RANGE1* descriptorRange);
	static std::vector<CD3DX12_ROOT_PARAMETER1> CreateRootParameters_Main(const D3D12_DESCRIPTOR_RANGE1* descriptorRange);
	static std::vector<CD3DX12_ROOT_PARAMETER1> CreateRootParameters_Debug();
	static UINT GetRootParameterNumValues(RootParameters paramType);
	static D3D12_SHADER_VISIBILITY GetRootParameterShaderVisibility(RootParameters paramType);

	static std::vector<std::pair<types::float3, types::float3>> s_debugAABBVertices;
	static std::vector<MaterialComponent::InputLayoutEntries> s_debugInputLayoutEntries;
	static constexpr const wchar_t* s_debugVSPath = L"data/shaders/VS_MvpColor.cso";
	static constexpr const wchar_t* s_debugPSPath = L"data/shaders/PS_Color.cso";
};

}
#endif
}
}