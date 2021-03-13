// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12ConstantBuffer.h"

#include "system/drivers/win/dx/DX12Driver.h"

#include "utils/snake_math.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

DX12ConstantBuffer::DX12ConstantBuffer(const std::wstring& name)
	: DX12Buffer(name)
	, m_sizeInBytes(0)
{
	DX12Driver* dx12Driver = (DX12Driver*)DX12Driver::GetInstance();
	m_constantBufferView = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DX12ConstantBuffer::CreateViews(size_t numElements, size_t elementSize)
{
	m_sizeInBytes = numElements * elementSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC d3d12ConstantBufferViewDesc;
	d3d12ConstantBufferViewDesc.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
	d3d12ConstantBufferViewDesc.SizeInBytes = static_cast<UINT>(math::AlignUp(m_sizeInBytes, 16));

	DX12Driver* dx12Driver = (DX12Driver*)DX12Driver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();

	device->CreateConstantBufferView(&d3d12ConstantBufferViewDesc, m_constantBufferView.GetDescriptorHandle());
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ConstantBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
{
	throw std::exception("ConstantBuffer::GetShaderResourceView should not be called.");
}
D3D12_CPU_DESCRIPTOR_HANDLE DX12ConstantBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
{
	throw std::exception("ConstantBuffer::GetUnorderedAccessView should not be called.");
}

}
#endif
}
}