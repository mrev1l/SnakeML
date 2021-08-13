// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"

#include "system/drivers/win/dx/pipeline/DX12RootSignature.h"
#include "system/drivers/win/dx/resource/DX12VertexBuffer.h"
#include "system/drivers/win/dx/resource/DX12Texture.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

class DX12RenderComponent : public ComponentBaseImpl<DX12RenderComponent>
{
public:
	virtual ~DX12RenderComponent() = default;
	ComponentType GetComponentType() const override { return ComponentType::DX12RenderComponent; }

	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	DX12VertexBuffer m_vertexBuffer;
	DX12RootSignature m_rootSignature;
	DX12Texture m_texture;

	// AABB Debug rendering TODO extract?
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_debugPipelineState;
	DX12VertexBuffer m_debugVertexBuffer;
	DX12RootSignature m_debugRootSignature;
};
REGISTER_TYPE(DX12RenderComponent);

}
#endif
}
