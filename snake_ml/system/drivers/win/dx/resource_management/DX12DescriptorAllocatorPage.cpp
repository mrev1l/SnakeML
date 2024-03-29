// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "DX12DescriptorAllocatorPage.h"

#include "system/drivers/win/dx/DX12Driver.h"
#include "system/drivers/win/dx/helpers/directX_utils.h"

#include "system/drivers/win/os/helpers/win_utils.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

DX12DescriptorAllocatorPage::DX12DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
	: m_heapType(type)
	, m_numDescriptorsInHeap(numDescriptors)
{
	auto device = ((DX12Driver*)DX12Driver::GetInstance())->GetD3D12Device();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = m_heapType;
	heapDesc.NumDescriptors = m_numDescriptorsInHeap;

	WinUtils::ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_d3d12DescriptorHeap)));

	m_baseDescriptor = m_d3d12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(m_heapType);
	m_numFreeHandles = m_numDescriptorsInHeap;

	// Initialize the free lists
	AddNewBlock(0, m_numFreeHandles);
}

bool DX12DescriptorAllocatorPage::HasSpace(uint32_t numDescriptors) const
{
	return m_freeListBySize.lower_bound(numDescriptors) != m_freeListBySize.end();
}

DX12DescriptorAllocation DX12DescriptorAllocatorPage::Allocate(uint32_t numDescriptors)
{
	std::lock_guard<std::mutex> lock(m_allocationMutex);

	// There are less than the requested number of descriptors left in the heap.
	// Return a NULL descriptor and try another heap.
	if (numDescriptors > m_numFreeHandles)
	{
		return DX12DescriptorAllocation();
	}

	// Get the first block that is large enough to satisfy the request.
	auto smallestBlockIt = m_freeListBySize.lower_bound(numDescriptors);
	if (smallestBlockIt == m_freeListBySize.end())
	{
		// There was no free block that could satisfy the request.
		return DX12DescriptorAllocation();
	}

	// The size of the smallest block that satisfies the request.
	auto blockSize = smallestBlockIt->first;

	// The pointer to the same entry in the FreeListByOffset map.
	auto offsetIt = smallestBlockIt->second;

	// The offset in the descriptor heap.
	auto offset = offsetIt->first;

	// Remove the existing free block from the free list.
	m_freeListBySize.erase(smallestBlockIt);
	m_freeListByOffset.erase(offsetIt);

	// Compute the new free block that results from splitting this block.
	auto newOffset = offset + numDescriptors;
	auto newSize = blockSize - numDescriptors;

	if (newSize > 0)
	{
		// If the allocation didn't exactly match the requested size,
		// return the left-over to the free list.
		AddNewBlock(newOffset, newSize);
	}

	// Decrement free handles.
	m_numFreeHandles -= numDescriptors;

	return DX12DescriptorAllocation(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(m_baseDescriptor, offset, m_descriptorHandleIncrementSize),
		numDescriptors, m_descriptorHandleIncrementSize, shared_from_this());
}

void DX12DescriptorAllocatorPage::Free(DX12DescriptorAllocation&& descriptorHandle, uint64_t frameNumber)
{
	// Compute the offset of the descriptor within the descriptor heap.
	auto offset = ComputeOffset(descriptorHandle.GetDescriptorHandle());

	std::lock_guard<std::mutex> lock(m_allocationMutex);

	// Don't add the block directly to the free list until the frame has completed.
	m_staleDescriptors.emplace(offset, descriptorHandle.GetNumHandles(), frameNumber);
}

void DX12DescriptorAllocatorPage::ReleaseStaleDescriptors(uint64_t frameNumber)
{
	std::lock_guard<std::mutex> lock(m_allocationMutex);

	while (!m_staleDescriptors.empty() && m_staleDescriptors.front().m_frameNumber <= frameNumber)
	{
		auto& staleDescriptor = m_staleDescriptors.front();

		// The offset of the descriptor in the heap.
		auto offset = staleDescriptor.m_offset;
		// The number of descriptors that were allocated.
		auto numDescriptors = staleDescriptor.m_size;

		FreeBlock(offset, numDescriptors);

		m_staleDescriptors.pop();
	}
}

uint32_t DX12DescriptorAllocatorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	return static_cast<uint32_t>(handle.ptr - m_baseDescriptor.ptr) / m_descriptorHandleIncrementSize;
}

void DX12DescriptorAllocatorPage::AddNewBlock(uint32_t offset, uint32_t numDescriptors)
{
	auto offsetIt = m_freeListByOffset.emplace(offset, numDescriptors);
	auto sizeIt = m_freeListBySize.emplace(numDescriptors, offsetIt.first);
	offsetIt.first->second.m_freeListBySizeIt = sizeIt;
}

void DX12DescriptorAllocatorPage::FreeBlock(uint32_t offset, uint32_t numDescriptors)
{
	// Find the first element whose offset is greater than the specified offset.
	// This is the block that should appear after the block that is being freed.
	auto nextBlockIt = m_freeListByOffset.upper_bound(offset);

	// Find the block that appears before the block being freed.
	auto prevBlockIt = nextBlockIt;
	// If it's not the first block in the list.
	if (prevBlockIt != m_freeListByOffset.begin())
	{
		// Go to the previous block in the list.
		--prevBlockIt;
	}
	else
	{
		// Otherwise, just set it to the end of the list to indicate that no
		// block comes before the one being freed.
		prevBlockIt = m_freeListByOffset.end();
	}

	// Add the number of free handles back to the heap.
	// This needs to be done before merging any blocks since merging
	// blocks modifies the numDescriptors variable.
	m_numFreeHandles += numDescriptors;

	if (prevBlockIt != m_freeListByOffset.end() &&
		offset == prevBlockIt->first + prevBlockIt->second.m_size)
	{
		// The previous block is exactly behind the block that is to be freed.
		//
		// PrevBlock.Offset		   Offset
		// |						  |
		// |<-----PrevBlock.Size----->|<------Size-------->|
		//

		// Increase the block size by the size of merging with the previous block.
		offset = prevBlockIt->first;
		numDescriptors += prevBlockIt->second.m_size;

		// Remove the previous block from the free list.
		m_freeListBySize.erase(prevBlockIt->second.m_freeListBySizeIt);
		m_freeListByOffset.erase(prevBlockIt);
	}

	if (nextBlockIt != m_freeListByOffset.end() &&
		offset + numDescriptors == nextBlockIt->first)
	{
		// The next block is exactly in front of the block that is to be freed.
		//
		// Offset			   NextBlock.Offset 
		// |					|
		// |<------Size-------->|<-----NextBlock.Size----->|

		// Increase the block size by the size of merging with the next block.
		numDescriptors += nextBlockIt->second.m_size;

		// Remove the next block from the free list.
		m_freeListBySize.erase(nextBlockIt->second.m_freeListBySizeIt);
		m_freeListByOffset.erase(nextBlockIt);
	}

	// Add the freed block to the free list.
	AddNewBlock(offset, numDescriptors);
}

}
#endif
}
 