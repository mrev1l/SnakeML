#pragma once
#include "DX12RenderCommand.h"

#include "lib_includes/directX_includes.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

class DX12RootSignature;

// TODO : rename / generalize
class SetupAABBPixelShaderCommand : public DX12RenderCommand
{
public:
	SetupAABBPixelShaderCommand(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, const DX12RootSignature& rootSignature, const math::matrix& mvp);
	~SetupAABBPixelShaderCommand() = default;

	void Execute(std::shared_ptr<DX12CommandList> commandList) override;

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	const DX12RootSignature& m_rootSignature;
	const math::matrix m_mvpMatrix;

};

}
#endif
}
}