// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "lib_includes/directX_includes.h"
#include "system/ecs/ISystem.h"

namespace snakeml
{
namespace system
{

#ifdef _WINDOWS

namespace win
{
	class DX12RootSignature;
}

class InitializeRenderComponentsSystem : public ISystem
{
public:
	void Execute() override;

private:
	static void CreateRootSignature(
		UINT inputLayout_num32BitValues,
		UINT inputLayout_shaderRegister,
		UINT inputLayout_registerSpace,
		D3D12_SHADER_VISIBILITY inputLayout_visibility,
		win::DX12RootSignature& _outRootSignature);
	static void CreatePipelineState(
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob,
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob,
		Microsoft::WRL::ComPtr<ID3D12PipelineState>& _outPipelineState);

};
#endif

}
}