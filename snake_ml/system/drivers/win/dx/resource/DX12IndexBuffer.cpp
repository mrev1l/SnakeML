// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12IndexBuffer.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

DX12IndexBuffer::DX12IndexBuffer(const std::wstring& name)
	: DX12Buffer(name)
	, m_numIndicies(0)
	, m_indexFormat(DXGI_FORMAT_UNKNOWN)
	, m_indexBufferView({})
{
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12IndexBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
{
	throw std::exception("IndexBuffer::GetShaderResourceView should not be called.");
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12IndexBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
{
	throw std::exception("IndexBuffer::GetUnorderedAccessView should not be called.");
}

void DX12IndexBuffer::CreateViews(size_t numElements, size_t elementSize)
{
	constexpr size_t two_byte_size = 2u, four_byte_size = 4u;
	ASSERT(elementSize == two_byte_size || elementSize == four_byte_size, "Indices must be 16, or 32-bit integers.");

	m_numIndicies = numElements;
	m_indexFormat = (elementSize == two_byte_size) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	m_indexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = static_cast<UINT>(numElements * elementSize);
	m_indexBufferView.Format = m_indexFormat;
}

}
#endif
}
}