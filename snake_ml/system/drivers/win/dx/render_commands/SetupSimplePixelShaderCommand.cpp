// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "SetupSimplePixelShaderCommand.h"

#include "system/drivers/win/dx/DX12CommandList.h"
#include "system/drivers/win/dx/DX12Driver.h"

namespace snakeml
{
namespace system
{
namespace win
{

SetupSimplePixelShaderCommand::SetupSimplePixelShaderCommand(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, const DX12RootSignature& rootSignature, const DirectX::XMMATRIX& mvp)
	: DX12RenderCommand()
	, m_pipelineState(pipelineState)
	, m_rootSignature(rootSignature)
	, m_mvpMatrix(mvp)
{
}

void SetupSimplePixelShaderCommand::Execute(std::shared_ptr<DX12CommandList> commandList)
{
	commandList->SetPipelineState(m_pipelineState);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->SetGraphics32BitConstants(RootParameters::MatricesCB, m_mvpMatrix);
}

}
}
}