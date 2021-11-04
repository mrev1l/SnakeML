#pragma once
#include "DX12RenderCommand.h"

#include "lib_includes/directX_includes.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

class DX12RootSignature;
class DX12Texture;

class SetupSimplePixelShaderCommand : public DX12RenderCommand
{
public:
	SetupSimplePixelShaderCommand(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, const DX12RootSignature& rootSignature, const DX12Texture& texture,
		const matrix& mvp, int32_t textureId);
	~SetupSimplePixelShaderCommand() = default;

	void Execute(std::shared_ptr<DX12CommandList> commandList) override;

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	const DX12RootSignature& m_rootSignature;
	const DX12Texture& m_texture;
	const matrix m_mvpMatrix;
	const int32_t m_textureId;
};

}
#endif
}
