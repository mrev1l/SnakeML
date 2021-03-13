// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12StructuredBuffer.h"

#include "system/drivers/win/dx/DX12Driver.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

DX12StructuredBuffer::DX12StructuredBuffer(const std::wstring& name)
	: DX12Buffer(name)
	, m_counterBuffer(CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 1, 4, name + L" Counter")
{
	DX12Driver* dx12Driver = (DX12Driver*)DX12Driver::GetInstance();
	m_SRV = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_UAV = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

DX12StructuredBuffer::DX12StructuredBuffer(const D3D12_RESOURCE_DESC& resDesc, size_t numElements, size_t elementSize, const std::wstring& name)
	: DX12Buffer(resDesc, numElements, elementSize, name)
	, m_counterBuffer(CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 1, 4, name + L" Counter")
	, m_numElements(numElements)
	, m_elementSize(elementSize)
{
	DX12Driver* dx12Driver = (DX12Driver*)DX12Driver::GetInstance();
	m_SRV = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_UAV = dx12Driver->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DX12StructuredBuffer::CreateViews(size_t numElements, size_t elementSize)
{
	DX12Driver* dx12Driver = (DX12Driver*)DX12Driver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();

	m_numElements = numElements;
	m_elementSize = elementSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = static_cast<UINT>(m_numElements);
	srvDesc.Buffer.StructureByteStride = static_cast<UINT>(m_elementSize);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_d3d12Resource.Get(),
		&srvDesc,
		m_SRV.GetDescriptorHandle());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.NumElements = static_cast<UINT>(m_numElements);
	uavDesc.Buffer.StructureByteStride = static_cast<UINT>(m_elementSize);
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	device->CreateUnorderedAccessView(m_d3d12Resource.Get(),
		m_counterBuffer.GetD3D12Resource().Get(),
		&uavDesc,
		m_UAV.GetDescriptorHandle());
}

}
#endif
}
}