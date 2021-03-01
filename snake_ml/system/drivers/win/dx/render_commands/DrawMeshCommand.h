#pragma once
#include "DX12RenderCommand.h"

namespace snakeml
{
namespace system
{
namespace win
{

class DX12VertexBuffer;

class DrawMeshCommand : public DX12RenderCommand
{
public:
	DrawMeshCommand(D3D_PRIMITIVE_TOPOLOGY topology, const DX12VertexBuffer& vertexBuffer);
	~DrawMeshCommand() = default;

	void Execute(std::shared_ptr<DX12CommandList> commandList) override;

private:
	D3D_PRIMITIVE_TOPOLOGY m_topology;
	const DX12VertexBuffer& m_vertexBuffer;
};

}
}
}