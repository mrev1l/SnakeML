// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12DescriptorAllocator.h"

#include "DX12DescriptorAllocatorPage.h"

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

DX12DescriptorAllocator::DX12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap)
	: m_heapType(type)
	, m_numDescriptorsPerHeap(numDescriptorsPerHeap)
{
}

DX12DescriptorAllocation DX12DescriptorAllocator::Allocate(uint32_t numDescriptors)
{
	std::lock_guard<std::mutex> lock(m_allocationMutex);

	DX12DescriptorAllocation allocation;

	for (auto iter = m_availableHeaps.begin(); iter != m_availableHeaps.end(); ++iter)
	{
		auto allocatorPage = m_heapPool[*iter];

		allocation = allocatorPage->Allocate(numDescriptors);

		if (allocatorPage->GetNumFreeHandles() == 0)
		{
			iter = m_availableHeaps.erase(iter);
		}

		// A valid allocation has been found.
		if (!allocation.IsNull())
		{
			break;
		}
	}

	// No available heap could satisfy the requested number of descriptors.
	if (allocation.IsNull())
	{
		m_numDescriptorsPerHeap = std::max(m_numDescriptorsPerHeap, numDescriptors);
		auto newPage = CreateAllocatorPage();

		allocation = newPage->Allocate(numDescriptors);
	}

	return allocation;
}

void DX12DescriptorAllocator::ReleaseStaleDescriptors(uint64_t frameNumber)
{
	std::lock_guard<std::mutex> lock(m_allocationMutex);

	for (size_t i = 0; i < m_heapPool.size(); ++i)
	{
		auto page = m_heapPool[i];

		page->ReleaseStaleDescriptors(frameNumber);

		if (page->GetNumFreeHandles() > 0)
		{
			m_availableHeaps.insert(i);
		}
	}
}

std::shared_ptr<DX12DescriptorAllocatorPage> DX12DescriptorAllocator::CreateAllocatorPage()
{
	auto newPage = std::make_shared<DX12DescriptorAllocatorPage>(m_heapType, m_numDescriptorsPerHeap); //-V106

	m_heapPool.emplace_back(newPage);
	m_availableHeaps.insert(m_heapPool.size() - 1);

	return newPage;
}

}
#endif
}
}