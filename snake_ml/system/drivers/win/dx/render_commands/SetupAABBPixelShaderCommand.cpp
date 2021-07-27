// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "SetupAABBPixelShaderCommand.h"

#include "system/drivers/win/dx/pipeline/DX12CommandList.h"
#include "system/drivers/win/dx/pipeline/DX12RootSignature.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

SetupAABBPixelShaderCommand::SetupAABBPixelShaderCommand(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, const DX12RootSignature& rootSignature, const math::matrix& mvp)
	: DX12RenderCommand()
	, m_pipelineState(pipelineState)
	, m_rootSignature(rootSignature)
	, m_mvpMatrix(mvp)
{
}

void SetupAABBPixelShaderCommand::Execute(std::shared_ptr<DX12CommandList> commandList)
{
	commandList->SetPipelineState(m_pipelineState);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->SetGraphics32BitConstants(0, m_mvpMatrix);
}

}
#endif
}
}