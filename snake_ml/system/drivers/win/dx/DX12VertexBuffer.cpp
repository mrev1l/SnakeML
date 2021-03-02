// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12VertexBuffer.h"

namespace snakeml
{
namespace system
{
namespace win
{

DX12VertexBuffer::DX12VertexBuffer(const std::wstring& name)
	: DX12Buffer(name)
	, m_numVertices(0)
	, m_vertexStride(0)
	, m_vertexBufferView({})
{
}

void DX12VertexBuffer::CreateViews(size_t numElements, size_t elementSize)
{
	m_numVertices = numElements;
	m_vertexStride = elementSize;

	m_vertexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = static_cast<UINT>(m_numVertices * m_vertexStride);
	m_vertexBufferView.StrideInBytes = static_cast<UINT>(m_vertexStride);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12VertexBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
{
	throw std::exception("VertexBuffer::GetShaderResourceView should not be called.");
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12VertexBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
{
	throw std::exception("VertexBuffer::GetUnorderedAccessView should not be called.");
}

}
}
}