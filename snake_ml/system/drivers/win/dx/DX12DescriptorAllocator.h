#pragma once
#pragma region (copyright)
/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file DescriptorAllocator.h
  *  @date October 22, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief This is an allocator for CPU visible descriptors.
  *  CPU visible descriptors must be copied to a GPU visible descriptor heap before
  *  being used in a shader. The DynamicDescriptorHeap class is used to upload
  *  CPU visible descriptors to a GPU visible descriptor heap.
  *
  *  Variable sized memory allocation strategy based on:
  *  http://diligentgraphics.com/diligent-engine/architecture/d3d12/variable-size-memory-allocations-manager/
  *  Date Accessed: May 9, 2018
  */
#pragma endregion

#include "DX12DescriptorAllocation.h"
#include "lib_includes/directX_includes.h"

#include <set>
#include <mutex>

namespace snakeml
{
namespace system
{
#ifdef _WINDOWS
namespace win
{

class DX12DescriptorAllocator
{
public:
	DX12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 16);// 256);
	virtual ~DX12DescriptorAllocator() = default;

	/**
	 * Allocate a number of contiguous descriptors from a CPU visible descriptor heap.
	 *
	 * @param numDescriptors The number of contiguous descriptors to allocate.
	 * Cannot be more than the number of descriptors per descriptor heap.
	 */
	DX12DescriptorAllocation Allocate(uint32_t numDescriptors = 1);

	/**
	 * When the frame has completed, the stale descriptors can be released.
	 */
	void ReleaseStaleDescriptors(uint64_t frameNumber);

private:
	using DescriptorHeapPool = std::vector<std::shared_ptr<DX12DescriptorAllocatorPage>>;

	// Create a new heap with a specific number of descriptors.
	std::shared_ptr<DX12DescriptorAllocatorPage> CreateAllocatorPage();

	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
	uint32_t m_numDescriptorsPerHeap;
	DescriptorHeapPool m_heapPool;
	// Indices of available heaps in the heap pool.
	std::set<size_t> m_availableHeaps;
	std::mutex m_allocationMutex;
};

}
#endif
}
}