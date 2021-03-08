// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "system/ecs/IComponent.h"
#include "system/drivers/win/dx/DX12VertexBuffer.h"
#include "system/drivers/win/dx/DX12RootSignature.h"
#include "system/drivers/win/dx/DX12Texture.h"

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
	win::DX12VertexBuffer m_vertexBuffer;
	win::DX12RootSignature m_rootSignature;
	win::DX12Texture m_texture;
};
REGISTER_TYPE(DX12RenderComponent);

}
}