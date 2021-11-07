// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "SetupSimplePixelShaderCommand.h"

#include "system/drivers/win/dx/pipeline/DX12CommandList.h"
#include "system/drivers/win/dx/pipeline/DX12RootSignature.h"
#include "system/drivers/win/dx/resource/DX12Texture.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

SetupSimplePixelShaderCommand::SetupSimplePixelShaderCommand(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, const DX12RootSignature& rootSignature, const DX12Texture& texture,
	const matrix& mvp, std::optional<uint32_t> textureId)
	: DX12RenderCommand()
	, m_pipelineState(pipelineState)
	, m_rootSignature(rootSignature)
	, m_texture(texture)
	, m_mvpMatrix(mvp)
	, m_textureId(textureId)
{
}

void SetupSimplePixelShaderCommand::Execute(std::shared_ptr<DX12CommandList> commandList)
{
	commandList->SetPipelineState(m_pipelineState);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->SetGraphics32BitConstants(RootParameters::MatricesCB, m_mvpMatrix);
	if (m_textureId.has_value())
	{
		commandList->SetGraphics32BitConstants(RootParameters::TextureId, m_textureId.value());
	}
	commandList->SetShaderResourceView(RootParameters::Textures, 0, m_texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

}
#endif
}
