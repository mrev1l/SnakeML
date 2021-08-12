// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12DescriptorAllocation.h"

#include "DX12DescriptorAllocatorPage.h"

#include "system/drivers/RenderDriver.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

DX12DescriptorAllocation::DX12DescriptorAllocation()
	: m_descriptor{ 0 }
	, m_numHandles(0)
	, m_descriptorSize(0)
	, m_page(nullptr)
{
}

DX12DescriptorAllocation::DX12DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DX12DescriptorAllocatorPage> page)
	: m_descriptor(descriptor)
	, m_numHandles(numHandles)
	, m_descriptorSize(descriptorSize)
	, m_page(page)
{
}

DX12DescriptorAllocation::~DX12DescriptorAllocation()
{
	Free();
}

DX12DescriptorAllocation::DX12DescriptorAllocation(DX12DescriptorAllocation&& allocation)
	: m_descriptor(allocation.m_descriptor)
	, m_numHandles(allocation.m_numHandles)
	, m_descriptorSize(allocation.m_descriptorSize)
	, m_page(std::move(allocation.m_page))
{
	allocation.m_descriptor.ptr = 0;
	allocation.m_numHandles = 0;
	allocation.m_descriptorSize = 0;
}

DX12DescriptorAllocation& DX12DescriptorAllocation::operator=(DX12DescriptorAllocation&& other)
{
	// Free this descriptor if it points to anything.
	Free();

	m_descriptor = other.m_descriptor;
	m_numHandles = other.m_numHandles;
	m_descriptorSize = other.m_descriptorSize;
	m_page = std::move(other.m_page);

	other.m_descriptor.ptr = 0;
	other.m_numHandles = 0;
	other.m_descriptorSize = 0;

	return *this;
}

bool DX12DescriptorAllocation::IsNull() const
{
	return m_descriptor.ptr == 0;;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorAllocation::GetDescriptorHandle(uint32_t offset) const
{
	ASSERT(offset < m_numHandles, "DX12DescriptorAllocation::GetDescriptorHandle");
	return { m_descriptor.ptr + static_cast<SIZE_T>(m_descriptorSize) * static_cast<SIZE_T>(offset) };
}

uint32_t DX12DescriptorAllocation::GetNumHandles() const
{
	return m_numHandles;
}

std::shared_ptr<DX12DescriptorAllocatorPage> DX12DescriptorAllocation::GetDescriptorAllocatorPage() const
{
	return m_page;
}

void DX12DescriptorAllocation::Free()
{
	if (!IsNull() && m_page)
	{
		m_page->Free(std::move(*this), IRenderDriver::GetInstance()->GetFrameCount());

		m_descriptor.ptr = 0;
		m_numHandles = 0;
		m_descriptorSize = 0;
		m_page.reset();
	}
}

}
#endif
}
