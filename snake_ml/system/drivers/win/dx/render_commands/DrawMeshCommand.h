#pragma once
#include "DX12RenderCommand.h"

#include "lib_includes/directX_includes.h"

namespace snakeml
{
#ifdef _WINDOWS
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
#endif
}
