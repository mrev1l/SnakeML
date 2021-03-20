// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/drivers/win/dx/helpers/directX_utils.h"
#include "system/drivers/win/dx/pipeline/DX12RootSignature.h"

#include "system/ecs/ISystem.h"
#include "system/ecs/components/MaterialComponent.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

class DX12RootSignature;

class InitializeRenderComponentsSystem : public ISystem
{
public:
	void Execute() override;

private:
	static void CreateRootSignature(DX12RootSignature& _outRootSignature);
	static void CreatePipelineState(
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob,
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob,
		Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outPipelineState);

	static DXGI_FORMAT GetInputLayoutFormat(MaterialComponent::InputLayoutEntries layoutEntry);
	static DX12Utils::DX12ShaderSemanticName GetShaderSemanticName(MaterialComponent::InputLayoutEntries layoutEntry);
	static void GenerateInputLayout(const std::vector<MaterialComponent::InputLayoutEntries>& inputLayoutEntries, std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout);
	static UINT GetRootParameterNumValues(RootParameters paramType);
	static D3D12_SHADER_VISIBILITY GetRootParameterShaderVisibility(RootParameters paramType);
};

}
#endif
}
}