// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DrawMeshCommand.h"

#include "system/drivers/win/dx/DX12CommandList.h"
#include "system/drivers/win/dx/Dx12VertexBuffer.h"

namespace snakeml
{
namespace system
{
namespace win
{

DrawMeshCommand::DrawMeshCommand(D3D_PRIMITIVE_TOPOLOGY topology, const DX12VertexBuffer& vertexBuffer)
	: DX12RenderCommand()
	, m_topology(topology)
	, m_vertexBuffer(vertexBuffer)
{
}

void DrawMeshCommand::Execute(std::shared_ptr<DX12CommandList> commandList)
{
	commandList->SetPrimitiveTopology(m_topology);
	commandList->SetVertexBuffer(0, m_vertexBuffer);
	commandList->Draw(m_vertexBuffer.GetNumVertices());
}

}
}
}