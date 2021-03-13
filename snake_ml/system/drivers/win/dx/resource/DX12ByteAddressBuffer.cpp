// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12ByteAddressBuffer.h"

#include "system/drivers/win/dx/DX12Driver.h"

#include "utils/snake_math.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

DX12ByteAddressBuffer::DX12ByteAddressBuffer(const std::wstring& name)
	: DX12Buffer(name)
{
	DX12Driver* dx12Driver = (DX12Driver*)DX12Driver::GetInstance();
	m_SRV = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_UAV = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

DX12ByteAddressBuffer::DX12ByteAddressBuffer(const D3D12_RESOURCE_DESC& resDesc, size_t numElements, size_t elementSize, const std::wstring& name)
	: DX12Buffer(resDesc, numElements, elementSize, name)
{
}

void DX12ByteAddressBuffer::CreateViews(size_t numElements, size_t elementSize)
{
	DX12Driver* dx12Driver = (DX12Driver*)DX12Driver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();

	// Make sure buffer size is aligned to 4 bytes.
	constexpr size_t alignment = 4u;
	m_bufferSize = math::AlignUp(numElements * elementSize, alignment);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = static_cast<UINT>(m_bufferSize / sizeof(UINT));
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	device->CreateShaderResourceView(m_d3d12Resource.Get(), &srvDesc, m_SRV.GetDescriptorHandle());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.Buffer.NumElements = static_cast<UINT>(m_bufferSize / sizeof(UINT));
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	device->CreateUnorderedAccessView(m_d3d12Resource.Get(), nullptr, &uavDesc, m_UAV.GetDescriptorHandle());
}

}
#endif
}
}