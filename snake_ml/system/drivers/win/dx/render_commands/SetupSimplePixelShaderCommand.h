#pragma once
#include "DX12RenderCommand.h"

namespace snakeml
{
namespace system
{
namespace win
{

class DX12RootSignature;

class SetupSimplePixelShaderCommand : public DX12RenderCommand
{
public:
	SetupSimplePixelShaderCommand(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, const DX12RootSignature& rootSignature, const DirectX::XMMATRIX& mvp);
	~SetupSimplePixelShaderCommand() = default;

	void Execute(std::shared_ptr<DX12CommandList> commandList) override;

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	const DX12RootSignature& m_rootSignature;
	const DirectX::XMMATRIX m_mvpMatrix;
};

}
}
}