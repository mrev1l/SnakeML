// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12UploadBuffer.h"

#include "system/drivers/win/dx/DX12Driver.h"

namespace snakeml
{
namespace system
{
namespace win
{

DX12UploadBuffer::DX12UploadBuffer(size_t pageSize) : m_pageSize(pageSize)
{
}

DX12UploadBuffer::Allocation DX12UploadBuffer::Allocate(size_t sizeInBytes, size_t alignment)
{
	if (sizeInBytes > m_pageSize)
	{
		throw std::bad_alloc();
	}

	// If there is no current page, or the requested allocation exceeds the
	// remaining space in the current page, request a new page.
	if (!m_currentPage || !m_currentPage->HasSpace(sizeInBytes, alignment))
	{
		m_currentPage = RequestPage();
	}

	return m_currentPage->Allocate(sizeInBytes, alignment);
}

void DX12UploadBuffer::Reset()
{
	m_currentPage = nullptr;
	// Reset all available pages.
	m_availablePages = m_pagePool;

	for (auto page : m_availablePages)
	{
		// Reset the page for new allocations.
		page->Reset();
	}
}

std::shared_ptr<DX12UploadBuffer::Page> DX12UploadBuffer::RequestPage()
{
	std::shared_ptr<Page> page;

	if (!m_availablePages.empty())
	{
		page = m_availablePages.front();
		m_availablePages.pop_front();
	}
	else
	{
		page = std::make_shared<Page>(m_pageSize);
		m_pagePool.push_back(page);
	}

	return page;
}

DX12UploadBuffer::Page::Page(size_t sizeInBytes)
	: m_pageSize(sizeInBytes)
	, m_offset(0)
	, m_CPUPtr(nullptr)
	, m_GPUPtr(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
	win::DX12Driver* dx12Driver = (win::DX12Driver*)IRenderDriver::GetInstance();
	auto device = dx12Driver->GetD3D12Device();

	const CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_pageSize);
	dxutils::ThrowIfFailed(device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_d3d12Resource)
	));

	m_GPUPtr = m_d3d12Resource->GetGPUVirtualAddress();
	m_d3d12Resource->Map(0, nullptr, &m_CPUPtr);
}

DX12UploadBuffer::Page::~Page()
{
	m_d3d12Resource->Unmap(0, nullptr);
	m_CPUPtr = nullptr;
	m_GPUPtr = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

bool DX12UploadBuffer::Page::HasSpace(size_t sizeInBytes, size_t alignment) const
{
	size_t alignedSize = math::AlignUp(sizeInBytes, alignment);
	size_t alignedOffset = math::AlignUp(m_offset, alignment);

	return alignedOffset + alignedSize <= m_pageSize;
}

DX12UploadBuffer::Allocation DX12UploadBuffer::Page::Allocate(size_t sizeInBytes, size_t alignment)
{
	if (!HasSpace(sizeInBytes, alignment))
	{
		// Can't allocate space from page.
		throw std::bad_alloc();
	}

	size_t alignedSize = math::AlignUp(sizeInBytes, alignment);
	m_offset = math::AlignUp(m_offset, alignment);

	Allocation allocation;
	allocation.CPU = static_cast<uint8_t*>(m_CPUPtr) + m_offset;
	allocation.GPU = m_GPUPtr + m_offset;

	m_offset += alignedSize;

	return allocation;
}

void DX12UploadBuffer::Page::Reset()
{
	m_offset = 0;
}

}
}
}