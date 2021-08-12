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
  *  @file DescriptorAllocation.h
  *  @date October 22, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief A single allocation for the descriptor allocator.
  *
  *  Variable sized memory allocation strategy based on:
  *  http://diligentgraphics.com/diligent-engine/architecture/d3d12/variable-size-memory-allocations-manager/
  *  Date Accessed: May 9, 2018
  */
#pragma endregion

#include "lib_includes/directX_includes.h"

namespace snakeml
{
#ifdef _WINDOWS
namespace win
{

class DX12DescriptorAllocatorPage;

class DX12DescriptorAllocation
{
public:
	// Creates a NULL descriptor.
	DX12DescriptorAllocation();
	DX12DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DX12DescriptorAllocatorPage> page);
	// The destructor will automatically free the allocation.
	~DX12DescriptorAllocation();
	
	DX12DescriptorAllocation(const DX12DescriptorAllocation&) = delete;
	DX12DescriptorAllocation& operator=(const DX12DescriptorAllocation&) = delete;

	DX12DescriptorAllocation(DX12DescriptorAllocation&& allocation);
	DX12DescriptorAllocation& operator=(DX12DescriptorAllocation&& other);

	bool IsNull() const;

	// Get a descriptor at a particular offset in the allocation.
	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0) const;

	// Get the number of (consecutive) handles for this allocation.
	uint32_t GetNumHandles() const;

	// Get the heap that this allocation came from.
	// (For internal use only).
	std::shared_ptr<DX12DescriptorAllocatorPage> GetDescriptorAllocatorPage() const;

private:
	// Free the descriptor back to the heap it came from.
	void Free();

	// The base descriptor.
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
	// The number of descriptors in this allocation.
	uint32_t m_numHandles;
	// The offset to the next descriptor.
	uint32_t m_descriptorSize;

	// A pointer back to the original page where this allocation came from.
	std::shared_ptr<DX12DescriptorAllocatorPage> m_page;
};

}
#endif
}
