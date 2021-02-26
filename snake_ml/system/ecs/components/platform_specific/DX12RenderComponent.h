// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"
#include "system/drivers/win/dx/DX12VertexBuffer.h"
#include "system/drivers/win/dx/DX12RootSignature.h"

namespace snakeml
{
namespace system
{

class DX12RenderComponent : public IComponent
{
public:
	virtual ~DX12RenderComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::DX12RenderComponent; }

	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	//Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	//Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	/*D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};*/

	win::DX12VertexBuffer m_vertexBuffer;
	win::DX12RootSignature m_rootSignature;
};
REGISTER_TYPE(DX12RenderComponent);

}
}